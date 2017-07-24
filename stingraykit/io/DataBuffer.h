#ifndef STINGRAYKIT_IO_DATABUFFER_H
#define STINGRAYKIT_IO_DATABUFFER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <string.h>
#include <deque>

#include <stingraykit/log/Logger.h>
#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	class BufferedDataConsumerBase : public virtual IDataConsumer
	{
	protected:
		static NamedLogger		s_logger;

		bool					_discardOnOverflow;
		BithreadCircularBuffer	_buffer;
		const size_t			_inputPacketSize;

		Mutex					_bufferMutex;
		Mutex					_writeMutex;

		ConditionVariable		_bufferEmpty;
		ConditionVariable		_bufferFull;
		bool					_eod;
		signal<void(size_t)>	_onOverflow;

	protected:
		BufferedDataConsumerBase(bool discardOnOverflow, size_t size, size_t inputPacketSize) :
			_discardOnOverflow(discardOnOverflow), _buffer(size),
			_inputPacketSize(inputPacketSize), _eod(false)
		{
			STINGRAYKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			STINGRAYKIT_CHECK(size % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
		}

		BufferedDataConsumerBase(bool discardOnOverflow, const BytesOwner& storage, size_t inputPacketSize) :
			_discardOnOverflow(discardOnOverflow), _buffer(storage),
			_inputPacketSize(inputPacketSize), _eod(false)
		{
			STINGRAYKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			STINGRAYKIT_CHECK(_buffer.GetTotalSize() % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
		}

	public:
		size_t GetDataSize() const		{ MutexLock l(_bufferMutex); return _buffer.GetDataSize(); }
		size_t GetFreeSize() const		{ MutexLock l(_bufferMutex); return _buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ MutexLock l(_bufferMutex); return _buffer.GetTotalSize(); }

		/// @brief: Clears buffer completely. Warning: can't be called simultaneously with Process(...) or Read(...)
		void Clear()
		{
			MutexLock l(_bufferMutex);
			_buffer.Clear();
			_eod = false;

			_bufferFull.Broadcast();
		}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			if (data.size() % _inputPacketSize != 0)
			{
				s_logger.Error() << "Data size: " << data.size() << " is not a multiple of input packet size: " << _inputPacketSize;
				return data.size();
			}

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			MutexLock l2(_bufferMutex);
			BithreadCircularBuffer::Writer w = _buffer.Write();
			size_t packetized_size = w.size() / _inputPacketSize * _inputPacketSize;
			if (packetized_size == 0)
			{
				if (_discardOnOverflow)
				{
					_onOverflow(data.size());
					return data.size();
				}

				_bufferFull.Wait(_bufferMutex, token);
				return 0;
			}

			size_t write_size = std::min(data.size(), packetized_size);
			{
				MutexUnlock ul(_bufferMutex);
				::memcpy(w.data(), data.data(), write_size);
			}

			w.Push(write_size);
			_bufferEmpty.Broadcast();

			return write_size;
		}

		virtual void EndOfData(const ICancellationToken&)
		{
			MutexLock l(_bufferMutex);
			_eod = true;
			_bufferEmpty.Broadcast();
		}

		signal_connector<void(size_t)> OnOverflow() const
		{ return _onOverflow.connector(); }
	};


	struct DataBufferBase : public virtual IDataBuffer, public BufferedDataConsumerBase
	{
		DataBufferBase(bool discardOnOverflow, size_t size, size_t inputPacketSize)
			: BufferedDataConsumerBase(discardOnOverflow, size, inputPacketSize)
		{ }

		DataBufferBase(bool discardOnOverflow, const BytesOwner& storage, size_t inputPacketSize)
			: BufferedDataConsumerBase(discardOnOverflow, storage, inputPacketSize)
		{ }

		virtual size_t GetDataSize() const                            { return BufferedDataConsumerBase::GetDataSize(); }
		virtual size_t GetFreeSize() const                            { return BufferedDataConsumerBase::GetFreeSize(); }
		virtual size_t GetStorageSize() const                         { return BufferedDataConsumerBase::GetStorageSize(); }

		virtual void Clear()                                          { BufferedDataConsumerBase::Clear(); }

		virtual signal_connector<void(size_t)> OnOverflow() const     { return BufferedDataConsumerBase::OnOverflow(); }
	};


	class DataBuffer : public DataBufferBase
	{
	private:
		static NamedLogger		s_logger;
		const size_t			_outputPacketSize;

	public:
		DataBuffer(bool discardOnOverflow, size_t size, size_t inputPacketSize = 1) :
			DataBufferBase(discardOnOverflow, size, inputPacketSize), _outputPacketSize(inputPacketSize)
		{ }

		DataBuffer(bool discardOnOverflow, size_t size, size_t inputPacketSize, size_t outputPacketSize) :
			DataBufferBase(discardOnOverflow, size, inputPacketSize), _outputPacketSize(outputPacketSize)
		{
			STINGRAYKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));
			STINGRAYKIT_CHECK(size % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

		DataBuffer(bool discardOnOverflow, const BytesOwner& storage, size_t inputPacketSize = 1) :
			DataBufferBase(discardOnOverflow, storage, inputPacketSize), _outputPacketSize(inputPacketSize)
		{ }

		DataBuffer(bool discardOnOverflow, const BytesOwner& storage, size_t inputPacketSize, size_t outputPacketSize) :
			DataBufferBase(discardOnOverflow, storage, inputPacketSize), _outputPacketSize(outputPacketSize)
		{
			STINGRAYKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));
			STINGRAYKIT_CHECK(_buffer.GetTotalSize() % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			MutexLock l(_bufferMutex);

			BithreadCircularBuffer::Reader r = _buffer.Read();

			size_t packetized_size = r.size() / _outputPacketSize * _outputPacketSize;
			if (packetized_size == 0)
			{
				if (_eod)
				{
					if (r.size() != 0)
						Logger::Warning() << "Dropping " << r.size() << " bytes from DataBuffer - end of data!";
					consumer.EndOfData(token);
					return;
				}

				_bufferEmpty.Wait(_bufferMutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(_bufferMutex);
				processed_size = consumer.Process(ConstByteData(r.GetData(), 0, packetized_size), token);
			}

			if (processed_size == 0)
				return;

			if (processed_size % _outputPacketSize != 0)
			{
				s_logger.Error() << "Processed size: " << processed_size << " is not a multiple of output packet size: " << _outputPacketSize;
				processed_size = packetized_size;
			}

			r.Pop(processed_size);
			_bufferFull.Broadcast();
		}
	};
	STINGRAYKIT_DECLARE_PTR(DataBuffer);


	template<typename MetadataType>
	class PacketBuffer :
		public virtual IPacketConsumer<MetadataType>, public virtual IPacketSource<MetadataType>
	{
		struct PacketInfo
		{
			size_t			Size;
			MetadataType	Metadata;

			PacketInfo(size_t size, const MetadataType& md) : Size(size), Metadata(md)
			{ }
		};

	private:
		static NamedLogger			s_logger;
		const bool					_discardOnOverflow;
		BithreadCircularBuffer		_buffer;
		std::deque<PacketInfo>		_packetQueue;

		Mutex						_bufferMutex;
		Mutex						_writeMutex;
		size_t						_paddingSize;

		ConditionVariable			_bufferEmpty;
		ConditionVariable			_bufferFull;
		bool						_eod;

	public:
		PacketBuffer(bool discardOnOverflow, size_t size) :
			_discardOnOverflow(discardOnOverflow), _buffer(size),
			_paddingSize(0), _eod(false)
		{ }

		size_t GetDataSize()			{ MutexLock l(_bufferMutex); return _buffer.GetDataSize(); }
		size_t GetFreeSize()			{ MutexLock l(_bufferMutex); return _buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ MutexLock l(_bufferMutex); return _buffer.GetTotalSize(); }

		void Clear()
		{
			MutexLock l(_bufferMutex);
			while (true)
			{
				BithreadCircularBuffer::Reader r = _buffer.Read();
				if (r.size() == 0)
					break;

				r.Pop(r.size());
			}
			_packetQueue.clear();
			_paddingSize = 0;
			_eod = false;

			_bufferFull.Broadcast();
		}

		virtual bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token)
		{
			ConstByteData data(packet.GetData());
			STINGRAYKIT_CHECK(data.size() <= GetStorageSize(), StringBuilder() % "Packet is too big! Buffer size: " % GetStorageSize() % " packet size:" % data.size());

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			MutexLock l2(_bufferMutex);

			BithreadCircularBuffer::Writer w = _buffer.Write();
			size_t padding_size = (w.size() < data.size() && w.IsBufferEnd()) ? w.size() : 0;
			if (_buffer.GetFreeSize() < padding_size + data.size())
			{
				if (_discardOnOverflow)
				{
					Logger::Warning() << "Overflow: dropping " << data.size() << " bytes";
					return true;
				}
				else
				{
					_bufferFull.Wait(_bufferMutex, token);
					return false;
				}
			}

			if (padding_size)
			{
				_paddingSize = padding_size;
				w.Push(padding_size);
				w = _buffer.Write();
			}

			{
				MutexUnlock ul(_bufferMutex);
				::memcpy(w.data(), data.data(), data.size());
			}
			PacketInfo p(data.size(), packet.GetMetadata());
			_packetQueue.push_back(p);

			w.Push(data.size());
			_bufferEmpty.Broadcast();

			return true;
		}

		virtual void EndOfData()
		{
			MutexLock l(_bufferMutex);
			_eod = true;
			_bufferEmpty.Broadcast();
		}

		virtual void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token)
		{
			MutexLock l(_bufferMutex);

			if (_packetQueue.empty())
			{
				if (_eod)
				{
					consumer.EndOfData();
					return;
				}

				_bufferEmpty.Wait(_bufferMutex, token);
				return;
			}

			BithreadCircularBuffer::Reader r = _buffer.Read();
			if (r.size() == _paddingSize && r.IsBufferEnd())
			{
				r.Pop(_paddingSize);
				_paddingSize = 0;
				r = _buffer.Read();
			}

			PacketInfo p = _packetQueue.front();
			STINGRAYKIT_CHECK(p.Size <= r.size(), "Not enough data in packet buffer, need: " + ToString(p.Size) + ", got: " + ToString(r.size()));
			bool processed = false;
			{
				MutexUnlock ul(_bufferMutex);
				processed = consumer.Process(Packet<MetadataType>(ConstByteData(r.GetData(), 0, p.Size), p.Metadata), token);
			}

			if (!processed)
				return;

			r.Pop(p.Size);
			_packetQueue.pop_front();
			_bufferFull.Broadcast();
		}
	};


}

#endif
