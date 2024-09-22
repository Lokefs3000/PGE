#pragma once

#include "RenderHardware/DeviceBuffer.hpp"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace pge
{
	class VkDeviceBuffer : public DeviceBuffer
	{
	private:
		VkBuffer m_Buffer = nullptr;
		VmaAllocation m_Memory = nullptr;

		uint32_t m_Side;
		uint8_t m_Stride;

		VkDevice m_RefDevice;
		VmaAllocator m_RefAllocator;
	public:
		VkDeviceBuffer(VkDevice device, VmaAllocator alloc, DeviceBufferCreateInfo& createInfo);
		~VkDeviceBuffer() override;
	};
}