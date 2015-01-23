#ifndef STINGRAYKIT_IO_DATAPACKETER_H
#define STINGRAYKIT_IO_DATAPACKETER_H


#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class DataPacketer : public virtual IPacketSource<EmptyType>
	{
	private:
		IDataSourcePtr			_source;
		size_t					_packetSize;

	public:
		DataPacketer(const IDataSourcePtr& source, size_t packetSize) : _source(source), _packetSize(packetSize)
		{ }


		virtual void Read(IPacketConsumer<EmptyType>& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(bind(&DataPacketer::Do, this, ref(consumer), _1, _2), token); }

	private:
		size_t Do(IPacketConsumer<EmptyType>& consumer, ConstByteData data, const ICancellationToken& token)
		{
			ConstByteData packet(data, 0, _packetSize);
			return consumer.Process(Packet<EmptyType>(packet), token) ? _packetSize : 0;
		}
	};


	class DataDepacketer : public virtual IDataSource
	{
		typedef IPacketSource<EmptyType> PacketSource;
		STINGRAYKIT_DECLARE_PTR(PacketSource);

	private:
		PacketSourcePtr	_source;

	public:
		DataDepacketer(const PacketSourcePtr& source) : _source(source)
		{ }


		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(bind(&DataDepacketer::Do, this, ref(consumer), _1, _2), token); }

	private:
		bool Do(IDataConsumer& consumer, const Packet<EmptyType>& packet, const ICancellationToken& token)
		{
			size_t offset = 0;
			while (token && offset < packet.GetSize())
				offset += consumer.Process(ConstByteData(packet.GetData(), offset), token);
			return true;
		}
	};

}

#endif