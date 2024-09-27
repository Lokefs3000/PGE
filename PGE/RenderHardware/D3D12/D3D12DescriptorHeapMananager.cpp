#include "pch.h"
#include "D3D12DescriptorHeapMananager.hpp"

#include <spdlog/spdlog.h>

#include "D3D12DescriptorHeap.hpp"
#include "Utility/MathUtility.hpp"

uint16_t pge::D3D12DescriptorHeapManager::SearchForFreeSpace(uint8_t heap, uint8_t size)
{
	std::unique_ptr<D3D12DescriptorHeap>& descriptorHeap = m_Heaps[heap];
	for (uint16_t i = 0; i < m_FreeSpace.size(); i++)
	{
		D3D12HeapAllocation& alloc = m_FreeSpace[i];
		if (alloc.Heap == heap && alloc.Used + size <= alloc.Capacity)
		{
			return i;
		}
	}

	return UINT16_MAX;
}

void pge::D3D12DescriptorHeapManager::AttemptDescriptorMerge(uint8_t heap)
{
	for (uint16_t i = 0; i < m_FreeSpace.size();)
	{
		if (i == 0 || i + 1 >= m_FreeSpace.size()) { i++; continue; }

		D3D12HeapAllocation& alloc = m_FreeSpace[i];
		if (alloc.Heap == heap && alloc.Used == 0)
		{
			m_FreeSpace[i - 1].Capacity += alloc.Capacity;
			m_FreeSpace.erase(m_FreeSpace.begin() + i);
		}
		else
			i++;
	}
}

pge::D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(ID3D12Device10* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	m_HeapType = type;
	m_Device = device;
}

pge::D3D12DescriptorHeapManager::~D3D12DescriptorHeapManager()
{
}

pge::D3D12HeapAllocation pge::D3D12DescriptorHeapManager::Allocate(uint8_t count)
{
	std::unique_lock lock(m_HeapMutex);

	count = MathUtility::RoundToPow2(count); //generally for packing and simplicity

	for (auto& heap : m_Heaps)
	{
		if (heap->GetAvailableSize() >= count)
		{
			uint8_t index = heap->GetIndex();
			uint16_t res = SearchForFreeSpace(index, count);

			//no available space found
			if (res == UINT16_MAX)
			{
				AttemptDescriptorMerge(index);
				res = SearchForFreeSpace(index, count);

				//short explanation: fuck
				if (res == UINT16_MAX)
					continue;
			}

			//was found continue..
			heap->AllocateSpace(count);

			D3D12HeapAllocation& allocation = m_FreeSpace[res];
			if (allocation.Used > 0) //split em
			{
				D3D12HeapAllocation newAlloc{
					.CPU = D3D12_CPU_DESCRIPTOR_HANDLE {.ptr = allocation.CPU.ptr + allocation.Used * heap->GetIncrementSize() },
					.GPU = D3D12_GPU_DESCRIPTOR_HANDLE { .ptr = allocation.GPU.ptr + allocation.Used },
					.Used = count,
					.Capacity = allocation.Capacity - allocation.Used,
					.Heap = index
				};

				allocation.Capacity = allocation.Used;

				m_FreeSpace.push_back(newAlloc);
				return newAlloc;
			}

			return allocation;
		}
	}
}

void pge::D3D12DescriptorHeapManager::Free(D3D12HeapAllocation& alloc)
{
	std::unique_lock lock(m_HeapMutex);

	std::unique_ptr<D3D12DescriptorHeap>& heap = m_Heaps[alloc.Heap];
	heap->FreeSpace(alloc.Used);

	if (heap->IsEmpty()) //if theres no descriptors kill the heap!
	{
		m_Heaps[alloc.Heap] = nullptr;
	}

	for (D3D12HeapAllocation& foundAlloc : m_FreeSpace)
	{
		if (foundAlloc.CPU.ptr == alloc.CPU.ptr && foundAlloc.GPU.ptr == alloc.GPU.ptr)
		{
			foundAlloc.Used = 0;
			return;
		}
	}

	spdlog::get("PGE")->warn("Failed to free allocation for: #{} descriptors! Packing may not be optimal..", alloc.Used);
}

void pge::D3D12DescriptorHeapManager::DataAllocate(D3D12HeapAllocation& alloc, uint8_t size)
{
	std::unique_lock lock(m_QueueMutex);
	m_CommandQueue.push(D3D12HeapDataCommand(D3D12HeapDataCommand::Type::Allocate, alloc, size));
}

void pge::D3D12DescriptorHeapManager::DataFree(D3D12HeapAllocation& alloc)
{
	std::unique_lock lock(m_QueueMutex);
	m_CommandQueue.push(D3D12HeapDataCommand(D3D12HeapDataCommand::Type::Free, alloc));
}

void pge::D3D12DescriptorHeapManager::FlushCommandQueue()
{
	if (m_CommandQueue.empty()) return;

	std::unique_lock lock(m_QueueMutex);
	while (!m_CommandQueue.empty())
	{
		D3D12HeapDataCommand& command = m_CommandQueue.front();
		switch (command.CommandType)
		{
		case D3D12HeapDataCommand::Type::Allocate:
		{
			D3D12HeapAllocation temporary = Allocate(command.Size);
			memcpy(&command.Allocation, &temporary, sizeof(D3D12HeapAllocation));
			break;
		}
		case D3D12HeapDataCommand::Type::Free:
		{
			Free(command.Allocation);
			break;
		}
		default: break;
		}

		m_CommandQueue.pop();
	}
}
