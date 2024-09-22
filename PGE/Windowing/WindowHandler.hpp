#pragma once

#include <vector>
#include <memory>

#undef CreateWindow

struct GLFWwindow;

namespace pge
{
	struct WindowCreateInfo;
	class Window;
	class InputPoller;
	class BaseRenderDevice;

	struct WindowDestructor
	{
		void operator()(Window* ptr);
	};

	class WindowHandler
	{
	private:
		std::vector<std::unique_ptr<Window, WindowDestructor>> m_Windows;

		InputPoller* m_RefInputPoller;

		friend struct WindowDestructor;
	public:
		WindowHandler();
		~WindowHandler();

		Window* CreateWindow(WindowCreateInfo&& ci);
		void DeleteWindow(Window* window);

		void AcquireSwapChain(Window* window, BaseRenderDevice* rd);

		GLFWwindow* GetHandle(Window* window);

		void Connect(InputPoller* inputPoller);
	};
}