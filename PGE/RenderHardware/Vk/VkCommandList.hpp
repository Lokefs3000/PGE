#pragma once

#include "RenderHardware/CommandList.hpp"

#include <Volk/volk.h>
#include <vector>

namespace pge
{
	class VkCommandList : public CommandList
	{
	private:
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphore;
		std::vector<VkSemaphore> m_RenderAvailableSemaphore;
		std::vector<VkFence> m_InFlightFences;

		uint32_t m_MaxFrameInFlight;
		uint32_t m_FrameInFlight;

		bool m_HasBegun;

		bool m_IsValid;

		VkInstance m_RefInstance;
		VkDevice m_RefDevice;
		VkCommandPool m_RefPool;

		void CleanupResources();
	public:
		VkCommandList(VkInstance instance, VkDevice device, VkCommandPool pool, CommandListCreateInfo& createInfo);
		~VkCommandList();

		void Begin() override;
		void Close() override;

		void Submit(VkQueue graphicsQueue);
	};
}