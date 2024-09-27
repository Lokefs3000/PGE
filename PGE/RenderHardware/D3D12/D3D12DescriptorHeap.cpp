#include "pch.h"
#include "D3D12DescriptorHeap.hpp"

#include "D3D12Utility.hpp"

pge::D3D12DescriptorHeap::D3D12DescriptorHeap(ID3D12Device10* device, uint8_t index, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	m_AvailableSize = 1024;
	m_Index = 0;
	m_IncrementSize = device->GetDescriptorHandleIncrementSize(heapType);

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = heapType;
		desc.NumDescriptors = m_AvailableSize;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_Heap.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create descriptor heap!");
			m_AvailableSize = 0;
		}
	}
}

pge::D3D12DescriptorHeap::~D3D12DescriptorHeap()
{
}

void pge::D3D12DescriptorHeap::AllocateSpace(uint8_t count)
{
	m_AvailableSize -= count;
}

void pge::D3D12DescriptorHeap::FreeSpace(uint8_t count)
{
	m_AvailableSize += count;
}

uint16_t pge::D3D12DescriptorHeap::GetAvailableSize()
{
	return m_AvailableSize;
}

uint8_t pge::D3D12DescriptorHeap::GetIncrementSize()
{
	return m_IncrementSize;
}

uint8_t pge::D3D12DescriptorHeap::GetIndex()
{
	return m_Index;
}

bool pge::D3D12DescriptorHeap::IsEmpty()
{
	return m_AvailableSize >= 1024;
}
