#include "pch.h"
#include "VkUtility.hpp"

#include <Volk/volk.h>
#include <spdlog/spdlog.h>

#define CaseError(en) case en: errMsg = #en; break;

void pge::VkUtility::PrettyPrint(VkResult res, const char* message)
{
	const char* errMsg = nullptr;
	switch (res)
	{
		CaseError(VK_SUCCESS)
			CaseError(VK_NOT_READY)
			CaseError(VK_TIMEOUT)
			CaseError(VK_EVENT_SET)
			CaseError(VK_EVENT_RESET)
			CaseError(VK_INCOMPLETE)
			CaseError(VK_ERROR_OUT_OF_HOST_MEMORY)
			CaseError(VK_ERROR_OUT_OF_DEVICE_MEMORY)
			CaseError(VK_ERROR_INITIALIZATION_FAILED)
			CaseError(VK_ERROR_DEVICE_LOST)
			CaseError(VK_ERROR_MEMORY_MAP_FAILED)
			CaseError(VK_ERROR_LAYER_NOT_PRESENT)
			CaseError(VK_ERROR_EXTENSION_NOT_PRESENT)
			CaseError(VK_ERROR_FEATURE_NOT_PRESENT)
			CaseError(VK_ERROR_INCOMPATIBLE_DRIVER)
			CaseError(VK_ERROR_TOO_MANY_OBJECTS)
			CaseError(VK_ERROR_FORMAT_NOT_SUPPORTED)
			CaseError(VK_ERROR_FRAGMENTED_POOL)
			CaseError(VK_ERROR_OUT_OF_POOL_MEMORY)
			CaseError(VK_ERROR_INVALID_EXTERNAL_HANDLE)
			CaseError(VK_ERROR_FRAGMENTATION)
			CaseError(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
			CaseError(VK_PIPELINE_COMPILE_REQUIRED)
			CaseError(VK_ERROR_SURFACE_LOST_KHR)
			CaseError(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
			CaseError(VK_SUBOPTIMAL_KHR)
			CaseError(VK_ERROR_OUT_OF_DATE_KHR)
			CaseError(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
			CaseError(VK_ERROR_VALIDATION_FAILED_EXT)
			CaseError(VK_ERROR_INVALID_SHADER_NV)
			CaseError(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
			CaseError(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
			CaseError(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
			CaseError(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
			CaseError(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
			CaseError(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
			CaseError(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
			CaseError(VK_ERROR_NOT_PERMITTED_KHR)
			CaseError(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
			CaseError(VK_THREAD_IDLE_KHR)
			CaseError(VK_THREAD_DONE_KHR)
			CaseError(VK_OPERATION_DEFERRED_KHR)
			CaseError(VK_OPERATION_NOT_DEFERRED_KHR)
			CaseError(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR)
			CaseError(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)
			CaseError(VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT)
	}

	spdlog::get("PGE")->error("{}\n    {}", message, errMsg);
}
