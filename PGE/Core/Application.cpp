#include "pch.h"
#include "Application.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef _WIN32
#undef CreateWindow
#endif

#include "Windowing/Window.hpp"
#include "Windowing/WindowHandler.hpp"
#include "Input/InputPoller.hpp"
#include "Level/SceneHandler.hpp"
#include "Components/ComponentHandler.hpp"
#include "RenderHardware/BaseRenderDevice.hpp"
#include "Graphics/Shaders/ShaderHandler.hpp"

#include "RenderHardware/D3D12/D3D12RenderDevice.hpp"

#include "ApplicationLayer.hpp"

static std::shared_ptr<spdlog::logger> s_Logger;

pge::Application::Application()
{
	s_Logger = spdlog::stdout_color_mt("PGE");

	glfwInit();

	m_WindowHandler = std::make_unique<WindowHandler>();
	m_InputPoller = std::make_unique<InputPoller>();
	m_SceneHandler = std::make_unique<SceneHandler>();
	m_ComponentHandler = std::make_unique<ComponentHandler>();
	m_RenderDevice = std::make_unique<D3D12GraphicsDevice>(RenderDeviceCreateInfo{ true });
	m_ShaderHandler = std::make_unique<ShaderHandler>();
	m_TFExecutor = std::make_unique<tf::Executor>();

	s_Logger->info("Using {} threads..", m_TFExecutor->num_workers());
}

pge::Application::~Application()
{
	m_Layers.clear();

	m_TFExecutor.reset();
	m_ComponentHandler.reset();
	m_SceneHandler.reset();
	m_InputPoller.reset();
	m_ShaderHandler.reset();
	m_WindowHandler.reset();
	m_RenderDevice.reset();

	glfwTerminate();

	s_Logger.reset();
}

void pge::Application::Run()
{
	{
		m_WindowHandler->Connect(m_InputPoller.get());
		m_ComponentHandler->Connect(m_SceneHandler.get());
		m_ShaderHandler->Connect(m_RenderDevice.get(), m_TFExecutor.get());
	}

	Window* primaryWindow = nullptr;

	{
		primaryWindow = m_WindowHandler->CreateWindow({
			1336,
			726,
			"ApplicationBasic - PGEv0"
			});

		m_InputPoller->RegisterInput(m_WindowHandler->GetHandle(primaryWindow));
		m_WindowHandler->AcquireSwapChain(primaryWindow, m_RenderDevice.get());
	}

	{
		m_InputPoller->LowMemory += std::bind(&ShaderHandler::LowMemory, m_ShaderHandler.get());
	}

	for (std::unique_ptr<ApplicationLayer>& layer : m_Layers)
	{
		layer->Setup();
	}

	while (!m_WantsExit)
	{
		m_InputPoller->PollInput();

		for (std::unique_ptr<ApplicationLayer>& layer : m_Layers)
			layer->Update();

		m_RenderDevice->PresentSwapChain(primaryWindow->GetSwapChain());

		//LAST also allow this to be disabled
		if (primaryWindow->IsClosed())
			m_WantsExit = true;
	}

	{
		m_WindowHandler->DeleteWindow(primaryWindow);
	}
}

pge::WindowHandler* pge::Application::GetWindowHandler()
{
	return m_WindowHandler.get();
}

pge::InputPoller* pge::Application::GetInputPoller()
{
	return m_InputPoller.get();
}

pge::SceneHandler* pge::Application::GetSceneHandler()
{
	return m_SceneHandler.get();
}

pge::ComponentHandler* pge::Application::GetComponentHandler()
{
	return m_ComponentHandler.get();
}

pge::BaseRenderDevice* pge::Application::GetRenderDevice()
{
	return m_RenderDevice.get();
}

pge::ShaderHandler* pge::Application::GetShaderHandler()
{
	return m_ShaderHandler.get();
}
