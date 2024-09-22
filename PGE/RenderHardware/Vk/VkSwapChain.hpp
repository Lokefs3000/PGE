#pragma once

#include "RenderHardware/SwapChain.hpp"

#include <Volk/volk.h>

namespace pge
{
	class VkCapabilities;
	struct QueueFamilyIndices;

	class VkSwapChain : public SwapChain
	{
	private:
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_SwapChain;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_Views;

		VkFormat m_Format;
		VkExtent2D m_Extent;

		uint32_t m_FrameIndex;
		bool m_NeedsNewFrame;

		VkInstance m_RefInstance;
		VkDevice m_RefDevice;

		bool m_IsValid;

		VkSurfaceFormatKHR QueryFormat(VkSurfaceCapabilitiesKHR& capabilities, VkPhysicalDevice physicalDevice, bool srgb);
		VkPresentModeKHR QueryPresentMode(VkSurfaceCapabilitiesKHR& capabilities, VkPhysicalDevice physicalDevice);
		VkExtent2D QuerySwapExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR& capabilities, VkPhysicalDevice physicalDevice);
	public:
		VkSwapChain(QueueFamilyIndices& indices, VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, SwapChainCreateInfo& createInfo);
		~VkSwapChain() override;

		void Present(VkQueue queue);
	};
}