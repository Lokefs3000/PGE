#include "pch.h"
#include "VkSwapChain.hpp"

#include "VkUtility.hpp"
#include "VkCapabilities.hpp"
#include "VkRenderDevice.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

VkSurfaceFormatKHR pge::VkSwapChain::QueryFormat(VkSurfaceCapabilitiesKHR& capabilities, VkPhysicalDevice physicalDevice, bool srgb)
{
	VkResult ret = VK_SUCCESS;

	std::vector<VkSurfaceFormatKHR> formats;
	VkUtility_ArrayWithArg(physicalDevice, vkGetPhysicalDeviceSurfaceFormatsKHR, m_Surface, formats);
	if (ret != VK_SUCCESS)
	{
		VkUtility::PrettyPrint(ret, "Failed to get supported formats for surface!");
		return { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
	}

	for (const auto& availableFormat : formats) {
		if (availableFormat.format == (srgb ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8A8_UNORM) && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	spdlog::get("PGE")->warn("Failed to get suitable format! Returning first entry..");
	return formats[0];
}

VkPresentModeKHR pge::VkSwapChain::QueryPresentMode(VkSurfaceCapabilitiesKHR& capabilities, VkPhysicalDevice physicalDevice)
{
	VkResult ret = VK_SUCCESS;

	std::vector<VkPresentModeKHR> presentModes;
	VkUtility_ArrayWithArg(physicalDevice, vkGetPhysicalDeviceSurfacePresentModesKHR, m_Surface, presentModes);
	if (ret != VK_SUCCESS)
	{
		VkUtility::PrettyPrint(ret, "Failed to get supported present modes for surface!");
		return VK_PRESENT_MODE_MAX_ENUM_KHR;
	}

	for (const auto& availablePresentMode : presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	spdlog::get("PGE")->warn("Failed to get suitable present mode! Returning using FIFO mode..");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D pge::VkSwapChain::QuerySwapExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR& capabilities, VkPhysicalDevice physicalDevice)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

pge::VkSwapChain::VkSwapChain(QueueFamilyIndices& indices, VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, SwapChainCreateInfo& argCreateInfo)
{
	m_RefInstance = instance;
	m_RefDevice = device;

	m_FrameIndex = 0;
	m_NeedsNewFrame = true;

	m_IsValid = false;

	//create surface
	{
		VkResult ret = glfwCreateWindowSurface(instance, argCreateInfo.Window, nullptr, &m_Surface);
		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to create surface for window!");
			return;
		}
	}

	//create swapchain
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		VkResult ret = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &capabilities);
		if (ret != VK_SUCCESS)
		{
			vkDestroySurfaceKHR(instance, m_Surface, nullptr);

			VkUtility::PrettyPrint(ret, "Failed to get surface capabilities!");
			return;
		}

		VkSurfaceFormatKHR surfaceFormat = QueryFormat(capabilities, physicalDevice, argCreateInfo.sRGB);
		VkPresentModeKHR presentMode = QueryPresentMode(capabilities, physicalDevice);
		VkExtent2D extent = QuerySwapExtent(argCreateInfo.Width, argCreateInfo.Height, capabilities, physicalDevice);

		uint32_t imageCount = std::max(capabilities.minImageCount + 1, argCreateInfo.ImageCount);
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			spdlog::get("PGE")->warn("Too many images requested for swapchain! Requested: {}, Max: {}", imageCount, capabilities.maxImageCount);
			imageCount = capabilities.maxImageCount;
		}

		argCreateInfo.ImageCount = imageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = { argCreateInfo.Width, argCreateInfo.Height };
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.imageArrayLayers = 1;
		createInfo.minImageCount = imageCount;

		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		ret = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain);
		if (ret != VK_SUCCESS)
		{
			vkDestroySurfaceKHR(instance, m_Surface, nullptr);

			VkUtility::PrettyPrint(ret, "Failed to create swapchain for window!");
			return;
		}

		m_Format = surfaceFormat.format;
		m_Extent = extent;
	}

	//get images
	{
		m_Images.resize(argCreateInfo.ImageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &argCreateInfo.ImageCount, m_Images.data());
	}

	//create views
	{
		m_Views.resize(argCreateInfo.ImageCount, nullptr);

		for (size_t i = 0; i < argCreateInfo.ImageCount; i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_Format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult ret = vkCreateImageView(device, &createInfo, nullptr, &m_Views[i]);
			if (ret != VK_SUCCESS)
			{
				for (size_t j = 0; j < argCreateInfo.ImageCount; j++)
				{
					if (m_Views[j] == nullptr) break;
					vkDestroyImageView(device, m_Views[j], nullptr);
				}

				vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
				vkDestroySurfaceKHR(instance, m_Surface, nullptr);

				VkUtility::PrettyPrint(ret, "Failed to create image view for swapchain!");
				return;
			}
		}
	}

	m_IsValid = true;
}

pge::VkSwapChain::~VkSwapChain()
{
	if (m_IsValid)
	{
		for (VkImageView view : m_Views)
			vkDestroyImageView(m_RefDevice, view, nullptr);
		vkDestroySwapchainKHR(m_RefDevice, m_SwapChain, nullptr);
		vkDestroySurfaceKHR(m_RefInstance, m_Surface, nullptr);
	}
}

void pge::VkSwapChain::Present(VkQueue queue)
{
	if (!m_IsValid) return;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain;
	presentInfo.pImageIndices = &m_FrameIndex;

	VkResult res = vkQueuePresentKHR(queue, &presentInfo);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
	{
		//Cleanup();
		//Create();
	}

	m_NeedsNewFrame = true;
}
