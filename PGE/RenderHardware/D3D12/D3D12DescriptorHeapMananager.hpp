#pragma once

#include <d3d12.h>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

namespace pge
{
	class D3D12DescriptorHeap;

	struct D3D12HeapAllocation
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CPU;
		D3D12_GPU_DESCRIPTOR_HANDLE GPU;

		uint8_t Used; //actually rounded to a power of 2 but it doesnt matter all too much in the end
		uint8_t Capacity;

		uint8_t Heap;
	};

	struct D3D12HeapDataCommand //ref count?
	{
		enum class Type
		{
			Allocate,
			Free
		};

		Type CommandType;
		uint8_t Size;
		D3D12HeapAllocation& Allocation;

		D3D12HeapDataCommand(Type type, D3D12HeapAllocation& alloc, uint8_t size = 0) : CommandType(type), Allocation(alloc), Size(size) {};
	};

	//re-ordering descriptors is alot harder because of references! Maybe bite the bullet and just make allocations pointers?
	//TODO: Consider changing allocations to pointers
	class D3D12DescriptorHeapManager
	{
	private:
		D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
		URef<ID3D12Device10> m_Device;

		std::mutex m_HeapMutex;
		std::vector<std::unique_ptr<D3D12DescriptorHeap>> m_Heaps;
		std::vector<D3D12HeapAllocation> m_FreeSpace;

		std::mutex m_QueueMutex;
		std::queue<D3D12HeapDataCommand> m_CommandQueue;

		uint16_t SearchForFreeSpace(uint8_t heap, uint8_t size);
		void AttemptDescriptorMerge(uint8_t heap);
	public:
		D3D12DescriptorHeapManager(ID3D12Device10* device, D3D12_DESCRIPTOR_HEAP_TYPE type);
		~D3D12DescriptorHeapManager();

		//unsafe
		D3D12HeapAllocation Allocate(uint8_t count);
		void Free(D3D12HeapAllocation& alloc);

		//safe
		void DataAllocate(D3D12HeapAllocation& alloc, uint8_t size);
		void DataFree(D3D12HeapAllocation& alloc);

		void FlushCommandQueue();
	};
}