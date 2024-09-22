#include "pch.h"
#include "WindowHandler.hpp"

#include "Window.hpp"

#include "Input/InputPoller.hpp"
#include "RenderHardware/BaseRenderDevice.hpp"
#include "RenderHardware/SwapChain.hpp"

#include <GLFW/glfw3.h>

static pge::WindowHandler* s_WindowHandler = nullptr;

pge::WindowHandler::WindowHandler()
{
	if (s_WindowHandler != nullptr)
		abort();
	s_WindowHandler = this;
}

pge::WindowHandler::~WindowHandler()
{
	m_Windows.clear();

	s_WindowHandler = nullptr;

	m_RefInputPoller = nullptr;
}

pge::Window* pge::WindowHandler::CreateWindow(WindowCreateInfo&& ci)
{
	m_Windows.push_back(std::unique_ptr<Window, WindowDestructor>(new Window(ci)));
	m_RefInputPoller->EventRecieved += std::bind(&Window::ProcessEvent, m_Windows.back().get(), std::placeholders::_1);
	return m_Windows.back().get();
}

void pge::WindowHandler::DeleteWindow(Window* window)
{
	for (size_t i = 0; i < m_Windows.size(); i++)
	{
		if (m_Windows[i].get() == window)
		{
			m_Windows.erase(m_Windows.begin() + i);
			break;
		}
	}
}

void pge::WindowHandler::AcquireSwapChain(Window* window, BaseRenderDevice* rd)
{
	int w, h;
	glfwGetFramebufferSize(window->m_Window, &w, &h);

	window->m_SwapChain = rd->CreateSwapChain({
		.Width = (uint32_t)w,
		.Height = (uint32_t)h,

		.ImageCount = 2,

		.sRGB = false,

		.Window = window->m_Window
		});
}

GLFWwindow* pge::WindowHandler::GetHandle(Window* window)
{
	return window->m_Window;
}

void pge::WindowHandler::Connect(InputPoller* inputPoller)
{
	m_RefInputPoller = inputPoller;
}

void pge::WindowDestructor::operator()(Window* ptr)
{
	s_WindowHandler->m_RefInputPoller->EventRecieved
		-= Signal<InputEvent&>::FnProc(std::bind(&Window::ProcessEvent, ptr, std::placeholders::_1));
	delete ptr;
}
