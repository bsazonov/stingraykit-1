#ifndef STINGRAYKIT_IO_IBYTESTREAMWITHOFFSETANDSIZE_H
#define STINGRAYKIT_IO_IBYTESTREAMWITHOFFSETANDSIZE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IByteStream.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	class ByteStreamWithOffsetAndSize : public IByteStream
	{
		IByteStreamPtr	_stream;
		const s64		_offset;
		const s64		_endOffset;
		s64				_position;

	public:
		ByteStreamWithOffsetAndSize(const IByteStreamPtr & stream, s64 offset, s64 size) : _stream(stream), _offset(offset), _endOffset(_offset + size), _position(0)
		{
			STINGRAYKIT_CHECK(_offset >= 0 && _offset < _endOffset, IndexOutOfRangeException(_offset, 0, _endOffset));
			_stream->Seek(offset + _position);
		}

		virtual u64 Tell() const
		{
			u64 tell = _stream->Tell();
			STINGRAYKIT_CHECK(tell >= (u64)_offset && tell <= (u64)_endOffset, IndexOutOfRangeException(tell, _offset, _endOffset + 1));
			u64 position = tell - _offset;
			STINGRAYKIT_CHECK(position == (u64)_position, LogicException(StringBuilder() % "real position is " % position % ", our is " % _position));
			return position;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			s64 newPosition;

			switch(mode)
			{
			case SeekMode::Begin:
				newPosition = _offset + offset;
				break;

			case SeekMode::Current:
				newPosition = (s64)Tell() + _offset + offset;
				break;

			case SeekMode::End:
				newPosition = _endOffset + offset;
				break;

			default:
				STINGRAYKIT_THROW(NotImplementedException());
			}

			STINGRAYKIT_CHECK(newPosition >= _offset && newPosition <= _endOffset, IndexOutOfRangeException(newPosition, _offset, _endOffset + 1));
			_stream->Seek(newPosition);
			_position = newPosition - _offset;
		}

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{
			s64 remaining = _endOffset - _offset - _position;
			STINGRAYKIT_CHECK(remaining >= 0, IndexOutOfRangeException(remaining, 0, 0));
			_stream->Seek(_offset + _position);
			u64 readed = _stream->Read(ByteData(data, 0, (size_t)std::min((s64)data.size(), remaining)), token);
			_position += (s64)readed;
			return readed;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			s64 remaining = _endOffset - _offset - _position;
			STINGRAYKIT_CHECK(remaining >= 0, IndexOutOfRangeException(remaining, 0, 0));
			STINGRAYKIT_CHECK(remaining != 0, "no space left in byte stream");
			_stream->Seek(_offset + _position);
			u64 written = _stream->Write(ConstByteData(data, 0, (size_t)std::min((s64)data.size(), remaining)), token);
			_position += (s64)written;
			return written;
		}
	};

}


#endif