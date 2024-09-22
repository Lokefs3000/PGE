#pragma once

enum VkResult;

namespace pge
{
#define VkUtility_Array(inst, fn, vc) \
	{ \
		uint32_t count = 0; \
		ret = fn(inst, &count, nullptr); \
		vc.resize(count); \
		if (ret == VK_SUCCESS) \
			ret = fn(inst, &count, vc.data()); \
	}
#define VkUtility_ArrayNoRet(inst, fn, vc) \
	{ \
		uint32_t count = 0; \
		fn(inst, &count, nullptr); \
		vc.resize(count); \
		fn(inst, &count, vc.data()); \
	}
#define VkUtility_ArrayWithArg(inst, fn, arg, vc) \
	{ \
		uint32_t count = 0; \
		fn(inst, arg, &count, nullptr); \
		vc.resize(count); \
		fn(inst, arg, &count, vc.data()); \
	}

	class VkUtility
	{
	public:
		static void PrettyPrint(VkResult res, const char* message);
		static uint8_t BitCount(uint32_t bits);
	};
}