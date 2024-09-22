#include "pch.h"
#include "VkCommandList.hpp"

#include "VkUtility.hpp"

void pge::VkCommandList::CleanupResources()
{
	for (size_t i = 0; i < m_MaxFrameInFlight; i++)
	{
		if (m_ImageAvailableSemaphore[i] != nullptr)
			vkDestroySemaphore(m_RefDevice, m_ImageAvailableSemaphore[i], nullptr);
		if (m_RenderAvailableSemaphore[i] != nullptr)
			vkDestroySemaphore(m_RefDevice, m_RenderAvailableSemaphore[i], nullptr);
		if (m_InFlightFences[i] != nullptr)
			vkDestroyFence(m_RefDevice, m_InFlightFences[i], nullptr);
	}

	vkFreeCommandBuffers(m_RefDevice, m_RefPool, 2, m_CommandBuffers.data());
}

pge::VkCommandList::VkCommandList(VkInstance instance, VkDevice device, VkCommandPool pool, CommandListCreateInfo& argCreateInfo)
{
	m_RefInstance = instance;
	m_RefDevice = device;
	m_RefPool = pool;

	m_MaxFrameInFlight = argCreateInfo.MaxFramesInFlight;
	m_FrameInFlight = 0;

	m_HasBegun = false;

	m_IsValid = false;

	//create command buffers
	{
		m_CommandBuffers.resize(argCreateInfo.MaxFramesInFlight, nullptr);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = argCreateInfo.MaxFramesInFlight;

		VkResult res = vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data());
		if (res != VK_SUCCESS)
		{
			CleanupResources();

			VkUtility::PrettyPrint(res, "Failed to allocate command buffers!");
			return;
		}
	}

	//create syncronization objects
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_ImageAvailableSemaphore.resize(argCreateInfo.MaxFramesInFlight, nullptr);
		m_RenderAvailableSemaphore.resize(argCreateInfo.MaxFramesInFlight, nullptr);
		m_InFlightFences.resize(argCreateInfo.MaxFramesInFlight, nullptr);

		for (size_t i = 0; i < argCreateInfo.MaxFramesInFlight; i++)
		{
			VkResult res = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphore[i]);
			if (res != VK_SUCCESS)
			{
				CleanupResources();

				VkUtility::PrettyPrint(res, "Failed to create image available semaphore!");
				return;
			}

			res = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_RenderAvailableSemaphore[i]);
			if (res != VK_SUCCESS)
			{
				CleanupResources();

				VkUtility::PrettyPrint(res, "Failed to create render available semaphore!");
				return;
			}

			res = vkCreateFence(device, &fenceCreateInfo, nullptr, &m_InFlightFences[i]);
			if (res != VK_SUCCESS)
			{
				CleanupResources();

				VkUtility::PrettyPrint(res, "Failed to create in flight fence!");
				return;
			}
		}
	}

	m_IsValid = true;
}

pge::VkCommandList::~VkCommandList()
{
}

void pge::VkCommandList::Begin()
{
	if (m_HasBegun) return;

	vkWaitForFences(m_RefDevice, 1, &m_InFlightFences[m_FrameInFlight], VK_TRUE, UINT64_MAX);
	vkResetFences(m_RefDevice, 1, &m_InFlightFences[m_FrameInFlight]);

	vkResetCommandBuffer(m_CommandBuffers[m_FrameInFlight], 0);

	m_HasBegun = true;
}

void pge::VkCommandList::Close()
{
	if (!m_HasBegun) return;

	m_HasBegun = false;
}

void pge::VkCommandList::Submit(VkQueue graphicsQueue)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore[m_FrameInFlight] };
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_FrameInFlight];

	VkSemaphore signalSemaphores[] = { m_RenderAvailableSemaphore[m_FrameInFlight] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkResult res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_InFlightFences[m_FrameInFlight]);
	if (res != VK_SUCCESS)
	{
		VkUtility::PrettyPrint(res, "Failed to submit command list to graphics queue!");
	}

	m_FrameInFlight++;
	m_FrameInFlight *= m_FrameInFlight < m_MaxFrameInFlight;
}
