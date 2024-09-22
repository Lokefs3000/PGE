#pragma once

#include "RenderHardware/BaseRenderDevice.hpp"

#include <Volk/volk.h>
#include <optional>
#include <memory>

namespace pge
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool IsComplete() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
	};

	class SwapChain;
	struct SwapChainCreateInfo;

	class VkRenderDevice : public BaseRenderDevice
	{
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_Device;
		QueueFamilyIndices m_Indices;
		VkDevice m_LogicalDevice;

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		VkCommandPool m_CommandPool;

		uint32_t AcquireBest(VkPhysicalDevice device, VkQueueFlags flags, const std::vector<VkQueueFamilyProperties> properties);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	public:
		VkRenderDevice(RenderDeviceCreateInfo&& createInfo);
		~VkRenderDevice() override;

		std::unique_ptr<SwapChain> CreateSwapChain(SwapChainCreateInfo&& createInfo) override;
		std::unique_ptr<CommandList> CreateCommandList(CommandListCreateInfo&& createInfo) override;

		void SubmitCommandList(CommandList* commandList) override;
		void PresentSwapChain(SwapChain* swapChain) override;
	};
}