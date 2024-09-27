#pragma once

#include <d3d12.h>
#include <vector>

#include "URef.hpp"

namespace pge
{
	class D3D12DescriptorHeap
	{
	private:
		URef<ID3D12DescriptorHeap> m_Heap;

		uint16_t m_AvailableSize;
		uint8_t m_IncrementSize;

		uint8_t m_Index;
	public:
		D3D12DescriptorHeap(ID3D12Device10* device, uint8_t index, D3D12_DESCRIPTOR_HEAP_TYPE heapType);
		~D3D12DescriptorHeap();

		void AllocateSpace(uint8_t count);
		void FreeSpace(uint8_t count);

		uint16_t GetAvailableSize();
		uint8_t GetIncrementSize();
		uint8_t GetIndex();

		bool IsEmpty();
	};
}