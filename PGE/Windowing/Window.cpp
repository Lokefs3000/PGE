#include "pch.h"
#include "Window.hpp"

#include <GLFW/glfw3.h>

#include "Input/InputPoller.hpp"
#include "RenderHardware/SwapChain.hpp"

void pge::Window::ProcessEvent(InputEvent& ev)
{
	if (ev.Id == *(uint64_t*)m_Window && ev.Type == InputEventType::WindowClosed)
		m_IsClosed = true;
}

pge::Window::Window(WindowCreateInfo& ci)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(ci.Width, ci.Height, ci.Title, nullptr, nullptr);

	m_IsClosed = false;
}

pge::Window::~Window()
{
	m_SwapChain.reset();
	glfwDestroyWindow(m_Window);
}

bool pge::Window::IsClosed()
{
	return m_IsClosed;
}

pge::SwapChain* pge::Window::GetSwapChain()
{
	return m_SwapChain.get();
}
