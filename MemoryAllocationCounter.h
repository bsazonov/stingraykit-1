#ifndef STINGRAY_TOOLKIT_MEMORYALLOCATIONCOUNTER_H
#define STINGRAY_TOOLKIT_MEMORYALLOCATIONCOUNTER_H

#include <stingray/toolkit/shared_ptr.h>

namespace stingray {
	struct IMemoryAllocationCountable
	{
		virtual ~IMemoryAllocationCountable() {}
		virtual size_t GetAllocatedMemory() const = 0;
	};

	struct MemoryAllocationCounter
	{
	private:
		size_t _allocatedMemory;
	public:
		MemoryAllocationCounter() : _allocatedMemory(0)
		{}
		void Add(size_t memory)								{ _allocatedMemory += memory; }
		void Remove(size_t memory)							{ _allocatedMemory -= memory; }
		template<typename T>
		void Add(const T &item)								{ _allocatedMemory += item.GetAllocatedMemory(); }
		template<typename T>
		void Remove(const T &item)							{ _allocatedMemory -= item.GetAllocatedMemory(); }
		template<typename T>
		void AddByPtr(const shared_ptr<T> &item)			{ _allocatedMemory += item->GetAllocatedMemory(); }
		template<typename T>
		void RemoveByPtr(const shared_ptr<T> &item)			{ _allocatedMemory -= item->GetAllocatedMemory(); }
		template<typename T>
		void Process(const T& item, CollectionOp event)
		{
			if (event == CollectionOp::Added)
				Add(item);
			if (event == CollectionOp::Removed)
				Remove(item);
		}
		size_t GetAllocatedMemory() const					{ return _allocatedMemory; }
	};
}

#endif
