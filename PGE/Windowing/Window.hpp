#pragma once

struct GLFWwindow;

namespace pge
{
	struct WindowCreateInfo
	{
		int Width;
		int Height;

		const char* Title;
	};

	struct InputEvent;
	class SwapChain;

	class Window
	{
	private:
		GLFWwindow* m_Window;
		std::unique_ptr<SwapChain> m_SwapChain;

		bool m_IsClosed;

		void ProcessEvent(InputEvent& ev);

		friend class WindowHandler;
		friend struct WindowDestructor;
	public:
		Window(WindowCreateInfo& ci);
		~Window();

		bool IsClosed();

		SwapChain* GetSwapChain();
	};
}