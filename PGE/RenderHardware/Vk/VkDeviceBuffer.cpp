#include "pch.h"
#include "VkDeviceBuffer.hpp"

#include "VkUtility.hpp"

pge::VkDeviceBuffer::VkDeviceBuffer(VkDevice device, VmaAllocator alloc, DeviceBufferCreateInfo& argCreateInfo)
{
	m_Side = argCreateInfo.Size;
	m_Stride = argCreateInfo.Stride;

	m_RefDevice = device;
	m_RefAllocator = alloc;

	{
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = argCreateInfo.Size;
		switch (argCreateInfo.Usage)
		{
		case BufferUsage::Vertex: createInfo.flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
		case BufferUsage::Index: createInfo.flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
		case BufferUsage::Staging: createInfo.flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
		case BufferUsage::Constant: createInfo.flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
		case BufferUsage::TransferDst: createInfo.flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
		case BufferUsage::TransferSrc: createInfo.flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
		default: VkUtility::PrettyPrint(VkResult::VK_ERROR_UNKNOWN, "Unkown usage specified for buffer creation!"); break;
		}
		createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

		//create with protected if not enabled default on instance!
		//purge video memory if it fails with some "out of memory" error and try again!
		VkResult res = vkCreateBuffer(device, &createInfo, nullptr, &m_Buffer);
		if (res != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(res, "Failed to create buffer!");
			return;
		}
	}

	{
		VmaAllocationCreateInfo createInfo{};
		switch (argCreateInfo.Access)
		{
		case BufferAccess::None: createInfo.flags = 0; break;
		case BufferAccess::Read:
		case BufferAccess::Write:
		case BufferAccess::ReadWrite: createInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT; break;
		default: VkUtility::PrettyPrint(VkResult::VK_ERROR_UNKNOWN, "Unkown access specified for buffer creation!"); break;
		}
		createInfo.usage = VMA_MEMORY_USAGE_AUTO;

		VmaAllocationInfo allocInfo{};

		VkResult res = vmaAllocateMemoryForBuffer(alloc, m_Buffer, &createInfo, &m_Memory, &allocInfo);
		if (res != VK_SUCCESS)
		{
			vkDestroyBuffer(device, m_Buffer, nullptr);
			m_Buffer = nullptr;

			VkUtility::PrettyPrint(res, "Failed to allocate buffer memory!");
			return;
		}
	}
}

pge::VkDeviceBuffer::~VkDeviceBuffer()
{
	if (m_Memory != nullptr && m_Buffer != nullptr)
		vmaDestroyBuffer(m_RefAllocator, m_Buffer, m_Memory);
}
