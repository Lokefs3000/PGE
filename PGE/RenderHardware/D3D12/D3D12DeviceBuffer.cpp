#include "pch.h"
#include "D3D12DeviceBuffer.hpp"

#include "D3D12Utility.hpp"

pge::D3D12DeviceBuffer::D3D12DeviceBuffer(ID3D12Device12* device, D3D12MA::Allocator* allocator, DeviceBufferCreateInfo& createInfo)
{
	m_Size = createInfo.Size;
	m_Stride = createInfo.Stride;
	m_ByteSize = createInfo.Size * createInfo.Stride;

	{
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = createInfo.Size * (size_t)createInfo.Stride;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc = { 1, 0 };
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		bool newDynamic = (createInfo.Dynamic || ((uint8_t)createInfo.Usage & (uint8_t)pge::BufferUsage::Staging) > 0);

		D3D12MA::ALLOCATION_DESC alloc{};
		alloc.Flags = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
		alloc.HeapType = newDynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = allocator->CreateResource(&alloc, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, m_Allocation.GetAddressOf(), IID_PPV_ARGS(m_Resource.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create a buffer!");
			return;
		}
	}
}

pge::D3D12DeviceBuffer::~D3D12DeviceBuffer()
{
	if (m_MapPointer != nullptr)
		Unmap({ 0 });
}

void* pge::D3D12DeviceBuffer::Map()
{
	if (m_MapPointer != nullptr)
		return m_MapPointer;

	HRESULT hr = m_Resource->Map(0, nullptr, &m_MapPointer);
	if (FAILED(hr))
		D3D12Utility::PrettyPrint(hr, "Failed to map buffer!");

	return m_MapPointer;
}

void pge::D3D12DeviceBuffer::Unmap(BufferRange&& written)
{
	if (m_MapPointer != nullptr)
	{
		m_Resource->Unmap(0, (const D3D12_RANGE*)&written);
		m_MapPointer = nullptr;
	}
}

size_t pge::D3D12DeviceBuffer::GetSize()
{
	return m_Size;
}

uint8_t pge::D3D12DeviceBuffer::GetStride()
{
	return m_Stride;
}

size_t pge::D3D12DeviceBuffer::GetByteSize()
{
	return m_ByteSize;
}

D3D12_GPU_VIRTUAL_ADDRESS pge::D3D12DeviceBuffer::GetGpuAddress()
{
	return m_Resource->GetGPUVirtualAddress();
}
