#include "pch.h"
#include "InputPoller.hpp"

#include <GLFW/glfw3.h>

pge::InputPoller::InputPoller()
{
	
}

pge::InputPoller::~InputPoller()
{
}

void pge::InputPoller::PollInput()
{
	glfwPollEvents();
}

void pge::InputPoller::RegisterInput(GLFWwindow* window)
{
	glfwSetWindowUserPointer(window, this);

	glfwSetWindowPosCallback(window, [](GLFWwindow* wn, int x, int y)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::WindowMove;
			ev.Id = *(uint64_t*)wn;
			ev.Window.Data1 = x;
			ev.Window.Data2 = y;

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetWindowSizeCallback(window, [](GLFWwindow* wn, int w, int h)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::WindowResize;
			ev.Id = *(uint64_t*)wn;
			ev.Window.Data1 = w;
			ev.Window.Data2 = h;

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* wn)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::WindowClosed;
			ev.Id = *(uint64_t*)wn;

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetKeyCallback(window, [](GLFWwindow* wn, int key, int scancode, int action, int mods)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::Keyboard;
			ev.Id = *(uint64_t*)wn;
			ev.Keyboard.Key = key;
			ev.Keyboard.Mod = mods;
			ev.Keyboard.State = action;

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* wn, int button, int action, int mods)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::MouseButton;
			ev.Id = *(uint64_t*)wn;
			ev.Button.Button = button;
			ev.Button.State = action;

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* wn, double x, double y)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::MouseMove;
			ev.Id = *(uint64_t*)wn;
			ev.Move.X = static_cast<float>(x);
			ev.Move.Y = static_cast<float>(y);

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetScrollCallback(window, [](GLFWwindow* wn, double x, double y)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::MouseWheel;
			ev.Id = *(uint64_t*)wn;
			ev.Wheel.X = static_cast<float>(x);
			ev.Wheel.Y = static_cast<float>(y);

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});

	glfwSetCharCallback(window, [](GLFWwindow* wn, uint32_t ch)
		{
			pge::InputEvent ev{};
			ev.Type = pge::InputEventType::MouseMove;
			ev.Id = *(uint64_t*)wn;
			ev.Text.CodePoint = ch;

			reinterpret_cast<pge::InputPoller*>(glfwGetWindowUserPointer(wn))->EventRecieved(ev);
		});
}
