#pragma once

#include "RenderHardware/DeviceBuffer.hpp"
#include "URef.hpp"

#include <d3d12.h>
#include <D3D12MemAlloc.h>

namespace pge
{
	class D3D12DeviceBuffer : public DeviceBuffer
	{
	private:
		URef<ID3D12Resource> m_Resource;
		URef<D3D12MA::Allocation> m_Allocation;

		void* m_MapPointer = nullptr;

		size_t m_Size;
		uint8_t m_Stride;
		size_t m_ByteSize;
	public:
		D3D12DeviceBuffer(ID3D12Device12* device, D3D12MA::Allocator* allocator, DeviceBufferCreateInfo& createInfo);
		~D3D12DeviceBuffer() override;

		void* Map() override;
		void Unmap(BufferRange&& written) override;

		size_t GetSize() override;
		uint8_t GetStride() override;
		size_t GetByteSize() override;

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress();
	};
}