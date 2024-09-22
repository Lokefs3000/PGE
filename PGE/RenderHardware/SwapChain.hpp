#pragma once

#include <cstdint>

struct GLFWwindow;

namespace pge
{
	struct SwapChainCreateInfo
	{
		uint32_t Width;
		uint32_t Height;

		uint32_t ImageCount;

		bool sRGB;

		GLFWwindow* Window;
	};

	class SwapChain
	{
	private:

	public:
		SwapChain();
		virtual ~SwapChain();
	};
}