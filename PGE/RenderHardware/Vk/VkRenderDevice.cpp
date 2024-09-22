#include "pch.h"
#include "VkRenderDevice.hpp"

#include <array>
#include <spdlog/spdlog.h>

#include "VkUtility.hpp"
#include "VkSwapChain.hpp"
#include "VkCommandList.hpp"

static std::array<const char*, 1> s_DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static std::array<const char*, 3> s_InstanceExtensionsWDebug = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
	"VK_KHR_win32_surface"
#endif
};

static std::array<const char*, 1> s_ValLayers = {
	"VK_LAYER_KHRONOS_validation"
};

typedef VkBool32(VKAPI_PTR* PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
static PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR = nullptr;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		spdlog::get("PGE")->debug("[{}]: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		spdlog::get("PGE")->info("[{}]: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		spdlog::get("PGE")->warn("[{}]: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		spdlog::get("PGE")->error("[{}]: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
	default: break;
	}

	return VK_FALSE;
}

uint32_t pge::VkRenderDevice::AcquireBest(VkPhysicalDevice device, VkQueueFlags flags, const std::vector<VkQueueFamilyProperties> properties)
{
	uint32_t i = 0;

	uint32_t targetIndex = UINT32_MAX;
	uint32_t targetFlags = 0xFFFFFFFF;

	for (const auto& queueFamily : properties)
	{
		if ((queueFamily.queueFlags && flags) == 0) continue;

		if (targetIndex == UINT32_MAX)
		{
			targetIndex = i;
			targetFlags = queueFamily.queueFlags;
		}
		else if (std::popcount(queueFamily.queueFlags) < std::popcount(targetFlags)) //when optimized should equal only 1 instruction "popcnt"
		{
			targetIndex = i;
			targetFlags = queueFamily.queueFlags;
		}

		i++;
	}

	return targetIndex;
}

pge::QueueFamilyIndices pge::VkRenderDevice::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices{};

	std::vector<VkQueueFamilyProperties> families;
	VkUtility_ArrayNoRet(device, vkGetPhysicalDeviceQueueFamilyProperties, families);

	uint32_t i = 0;

	for (const auto& queueFamily : families)
	{
		VkBool32 presentSupport = vkGetPhysicalDeviceWin32PresentationSupportKHR(device, i);
		if (presentSupport)
		{
			indices.PresentFamily = i;
			break;
		}

		i++;
	}

	indices.GraphicsFamily = AcquireBest(device, VK_QUEUE_GRAPHICS_BIT, families);

	return indices;
}

pge::VkRenderDevice::VkRenderDevice(RenderDeviceCreateInfo&& argCreateInfo)
{
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback
	};

	//volk1
	{
		VkResult ret = volkInitialize();
		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to initialize Volk!");
			exit((int)ret);
		}
	}

	//instance
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Unkown";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
		appInfo.pEngineName = "PGE";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		if (argCreateInfo.DebuggingEnabled)
		{
			createInfo.pNext = &debugMessengerCreateInfo;

			createInfo.enabledExtensionCount = s_InstanceExtensionsWDebug.size();
			createInfo.ppEnabledExtensionNames = s_InstanceExtensionsWDebug.data();

			createInfo.enabledLayerCount = s_ValLayers.size();
			createInfo.ppEnabledLayerNames = s_ValLayers.data();
		}

		VkResult ret = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		if (ret == VK_ERROR_LAYER_NOT_PRESENT && argCreateInfo.DebuggingEnabled)
		{
			createInfo.ppEnabledLayerNames = 0;
			ret = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		}

		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to create a usable Vulkan instance!");
			exit((int)ret);
		}
	}

	//volk2
	{
		volkLoadInstance(m_Instance);
	}

	//setup debug messenger (if enabled)
	if (argCreateInfo.DebuggingEnabled)
	{
		VkResult ret = vkCreateDebugUtilsMessengerEXT(m_Instance, &debugMessengerCreateInfo, nullptr, &m_DebugMessenger);
		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to create a debug messenger!");
		}
	}

	//custom
	{
		HMODULE mod = LoadLibraryA("vulkan-1.dll");

		vkGetPhysicalDeviceWin32PresentationSupportKHR =
			(PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)GetProcAddress(mod, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
		if (vkGetPhysicalDeviceWin32PresentationSupportKHR == nullptr)
		{
			FreeLibrary(mod);

			VkUtility::PrettyPrint(VK_ERROR_FEATURE_NOT_PRESENT, "Failed to get address for function: vkGetPhysicalDeviceWin32PresentationSupportKHR!");
			exit((int)VK_ERROR_FEATURE_NOT_PRESENT);
		}

		FreeLibrary(mod);
	}

	//physical device
	{
		VkResult ret;

		std::vector<VkPhysicalDevice> physicalDevices{};
		VkUtility_Array(m_Instance, vkEnumeratePhysicalDevices, physicalDevices);
		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to enumerate physical devices!");
			exit((int)ret);
		}

		std::multimap<uint32_t, VkPhysicalDevice> scoreMap;
		for (VkPhysicalDevice& device : physicalDevices)
		{
			//check suitability
			bool suitable = true;

			QueueFamilyIndices indices = FindQueueFamilies(device);
			suitable = indices.IsComplete() && suitable;

			if (suitable)
			{
				//calculate score!
				uint32_t score = 0;

				scoreMap.insert(std::make_pair(score, device));
			}
		}

		if (scoreMap.empty())
		{
			VkUtility::PrettyPrint(VK_ERROR_UNKNOWN, "Failed to find any suitable physical device!");
			exit((int)VK_ERROR_UNKNOWN);
		}
		else
		{
			m_Device = scoreMap.rbegin()->second;
		}

		m_Indices = FindQueueFamilies(m_Device);
	}

	//device
	{
		std::vector<const char*> extensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());

		VkResult ret = VK_SUCCESS;

		std::vector<VkExtensionProperties> properties{};
		VkUtility_ArrayWithArg(m_Device, vkEnumerateDeviceExtensionProperties, nullptr, properties);
		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to get extensions for device!");
		}
		else
		{
			uint32_t supported = 0;

			for (VkExtensionProperties& extension : properties)
			{
				if (std::strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
				{
					extensions.push_back("VK_KHR_portability_subset");
					supported++;

					spdlog::get("PGE")->info("Adding device extension: \"VK_KHR_portability_subset\"..");
				}
				else if (std::strcmp(extension.extensionName, "VK_EXT_pageable_device_local_memory") == 0)
				{
					extensions.push_back("VK_EXT_pageable_device_local_memory");
					extensions.push_back("VK_EXT_memory_priority");

					supported += 2;

					spdlog::get("PGE")->info("Adding device extension: \"VK_EXT_pageable_device_local_memory\"..");
					spdlog::get("PGE")->info("Adding device extension: \"VK_EXT_memory_priority\"..");
				}
				else
				{
					for (const char* comparator : extensions)
					{
						if (std::strcmp(extension.extensionName, comparator) == 0)
						{
							supported++;
						}
					}
				}
			}

			if (supported < extensions.size())
			{
				VkUtility::PrettyPrint(VK_ERROR_EXTENSION_NOT_PRESENT, "Not all required device extensions were available!");
				exit((int)VK_ERROR_EXTENSION_NOT_PRESENT);
			}
		}

		float priorities = 0.0f;

		std::array<VkDeviceQueueCreateInfo, 2> queues{};
		queues[0] = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.flags = 0,
			.queueFamilyIndex = m_Indices.GraphicsFamily.value(),
			.queueCount = 1,
			.pQueuePriorities = &priorities
		};
		queues[1] = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.flags = 0,
			.queueFamilyIndex = m_Indices.PresentFamily.value(),
			.queueCount = 1,
			.pQueuePriorities = &priorities
		};

		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(m_Device, &features); //wack?

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queues.data();
		createInfo.queueCreateInfoCount = 2;
		createInfo.pEnabledFeatures = &features;
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledExtensionCount = extensions.size();

		ret = vkCreateDevice(m_Device, &createInfo, nullptr, &m_LogicalDevice);
		if (ret != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(ret, "Failed to create logical device!");
			exit((int)ret);
		}
	}

	//volk3
	{
		volkLoadDevice(m_LogicalDevice);
	}

	//queues
	{
		vkGetDeviceQueue(m_LogicalDevice, m_Indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_Indices.PresentFamily.value(), 0, &m_PresentQueue);
	}

	//command pool
	{
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = m_Indices.GraphicsFamily.value();

		VkResult res = vkCreateCommandPool(m_LogicalDevice, &createInfo, nullptr, &m_CommandPool);
		if (res != VK_SUCCESS)
		{
			VkUtility::PrettyPrint(res, "Failed to create command pool!");
			exit((int)res);
		}
	}

	//debug
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_Device, &properties);

		spdlog::get("PGE")->info("Rendering device overview:");
		spdlog::get("PGE")->info("       API: Vulkan {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
		spdlog::get("PGE")->info("    Driver: {}", properties.driverVersion);
		spdlog::get("PGE")->info("    Vendor: {}", properties.vendorID);
		spdlog::get("PGE")->info("    Device: {}", properties.deviceID);
		spdlog::get("PGE")->info("      Name: {}", properties.deviceName);
	}
}

pge::VkRenderDevice::~VkRenderDevice()
{
	vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
	vkDestroyDevice(m_LogicalDevice, nullptr);
	if (m_DebugMessenger != nullptr) vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

std::unique_ptr<pge::SwapChain> pge::VkRenderDevice::CreateSwapChain(SwapChainCreateInfo&& createInfo)
{
	return std::make_unique<VkSwapChain>(m_Indices, m_Instance, m_LogicalDevice, m_Device, createInfo);
}

std::unique_ptr<pge::CommandList> pge::VkRenderDevice::CreateCommandList(CommandListCreateInfo&& createInfo)
{
	return std::make_unique<VkCommandList>(m_Instance, m_LogicalDevice, m_CommandPool, createInfo);
}

void pge::VkRenderDevice::SubmitCommandList(CommandList* commandList)
{
	((VkCommandList*)commandList)->Submit(m_GraphicsQueue);
}

void pge::VkRenderDevice::PresentSwapChain(SwapChain* swapChain)
{
	((VkSwapChain*)swapChain)->Present(m_PresentQueue);
}
