#ifndef STINGRAY_TOOLKIT_IO_PAGEDBUFFER_H
#define STINGRAY_TOOLKIT_IO_PAGEDBUFFER_H

#include <deque>

#include <stingray/toolkit/ScopeExit.h>
#include <stingray/toolkit/collection/ByteData.h>

namespace stingray
{


	class PagedBuffer
	{
	public:
		struct IPage
		{
			virtual ~IPage() { }

			virtual size_t Read(u64 offset, ByteData data) = 0;
			virtual size_t Write(u64 offset, ConstByteData data) = 0;
		};
		typedef IPage Page;
		STINGRAYKIT_DECLARE_PTR(Page);

	private:
		typedef std::deque<PagePtr> PagesContainer;

	private:
		u64				_pageSize;
		PagesContainer	_pages;
		u64				_startOffset, _endOffset, _popOffset;
		Mutex			_mutex;
		bool			_pushing, _usingStart;

	public:
		PagedBuffer(u64 pageSize) :
			_pageSize(pageSize),
			_startOffset(0),
			_endOffset(0),
			_popOffset(0),
			_pushing(false),
			_usingStart(false)
		{ }

		virtual ~PagedBuffer()
		{ }

		void Push(const ConstByteData& data)
		{
			{
				MutexLock l(_mutex);
				STINGRAYKIT_CHECK(!_pushing, "Previous push has not finished yet!");
				_pushing = true;
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::PushingFinished, this));

			u64 new_end_offset, page_idx = 0, page_write_size, page_offset;
			{
				page_write_size = std::min(_endOffset, (u64)data.size());
				page_offset = _endOffset == 0 ? 0 : _pageSize - _endOffset;

				for (; data.size() > _endOffset; _endOffset += _pageSize, ++page_idx)
					_pages.push_back(CreatePage(_pageSize));

				new_end_offset = _endOffset - data.size();
			}
			ScopeExitInvoker sei2(bind(&PagedBuffer::SetEndOffset, this, new_end_offset));

			WriteToPage(page_idx--, page_offset, ConstByteData(data, 0, page_write_size));

			for (u64 data_offset = page_write_size; data_offset < data.size(); data_offset += page_write_size, --page_idx)
			{
				page_write_size = std::min(_pageSize, (u64)data.size() - data_offset);
				WriteToPage(page_idx, 0, ConstByteData(data, data_offset, page_write_size));
			}
		}

		void Get(const ByteData& data)
		{
			MutexLock l(_mutex);

			u64 page_idx = _startOffset / _pageSize, page_read_size, page_offset;
			{
				STINGRAYKIT_CHECK(data.size() <= GetSize(), IndexOutOfRangeException());

				STINGRAYKIT_CHECK(!_usingStart, "End is being used!");
				_usingStart = true;

				page_offset = _startOffset % _pageSize;
				page_read_size = std::min(_pageSize - _startOffset % _pageSize, (u64)data.size());
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::ReleaseStart, this));

			u64 data_offset = 0;
			ReadFromPage(page_idx++, page_offset, ByteData(data, data_offset, page_read_size));
			data_offset += page_read_size;

			for (; data_offset < data.size(); data_offset += page_read_size, ++page_idx)
			{
				page_read_size = std::min(_pageSize, data.size() - data_offset);
				ReadFromPage(page_idx, 0, ByteData(data, data_offset, page_read_size));
			}

			_startOffset += data.size();
		}

		void Seek(u64 offset)
		{
			MutexLock l(_mutex);
			STINGRAYKIT_CHECK(offset <= _pageSize * _pages.size() - _endOffset, IndexOutOfRangeException());

			_startOffset = _popOffset + offset;
		}

		void Pop(u64 size)
		{
			MutexLock l(_mutex);

			STINGRAYKIT_CHECK(size <= _pageSize * _pages.size(), IndexOutOfRangeException());
			STINGRAYKIT_CHECK(!_usingStart, "End is being used!");

			SetPopOffset(_popOffset + size);
		}

		u64 GetSize(bool absolute = false) const
		{
			MutexLock l(_mutex);

			if (absolute)
				return _pageSize * _pages.size() - _endOffset;

			return _pageSize * _pages.size() - _startOffset - _endOffset;
		}

		signal<void()> OnDiscontinuity;

	protected:
		virtual PagePtr CreatePage(u64 size) = 0;
		virtual void GCPage(PagePtr page) {}

	private:
		void PushingFinished()
		{
			MutexLock l(_mutex);
			_pushing = false;
		}

		void SetEndOffset(u64 newEndOffset)
		{
			MutexLock l(_mutex);
			_endOffset = newEndOffset;
		}

		void ReleaseStart()
		{
			MutexLock l(_mutex);
			_usingStart = false;
		}

		void SetPopOffset(u64 newPopOffset)
		{
			MutexLock l(_mutex);
			_popOffset = newPopOffset;

			for (; _popOffset >= _pageSize; _popOffset -= _pageSize)
			{
				GCPage(_pages.front());
				_pages.pop_front();

				if (_pageSize >= _startOffset)
				{
					_startOffset = 0;
					OnDiscontinuity();
				}
				else
					_startOffset -= _pageSize;
			}
		}

		void WriteToPage(u64 pageIdxFromEnd, u64 offsetInPage, ConstByteData data)
		{
			if (data.empty())
				return;

			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(_pages.size() - pageIdxFromEnd - 1);
			}
			if (p->Write(offsetInPage, data) != data.size())
				STINGRAYKIT_THROW("Page write failed!");
		}

		void ReadFromPage(u64 pageIdxFromStart, u64 offsetInPage, ByteData data) const
		{
			if (data.empty())
				return;

			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(pageIdxFromStart);
			}
			if (p->Read(offsetInPage, data) != data.size())
				STINGRAYKIT_THROW("Page read failed!");
		}
	};


}


#endif

