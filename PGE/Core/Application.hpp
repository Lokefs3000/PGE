#pragma once

#include <memory>
#include <vector>

namespace tf { class Executor; }

namespace pge
{
	class WindowHandler;
	class InputPoller;
	class SceneHandler;
	class ComponentHandler;
	class BaseRenderDevice;
	class ShaderHandler;

	class ApplicationLayer;

	class Window;

	class Application
	{
	private:
		std::unique_ptr<WindowHandler> m_WindowHandler;
		std::unique_ptr<InputPoller> m_InputPoller;
		std::unique_ptr<SceneHandler> m_SceneHandler;
		std::unique_ptr<ComponentHandler> m_ComponentHandler;
		std::unique_ptr<BaseRenderDevice> m_RenderDevice;
		std::unique_ptr<ShaderHandler> m_ShaderHandler; //subhandler of ResourceHandler?
		std::unique_ptr<tf::Executor> m_TFExecutor;

		std::vector<std::unique_ptr<ApplicationLayer>> m_Layers;

		bool m_WantsExit = false;
	public:
		Application();
		~Application();

		void Run();

		//GIVE ownership to application aka this will not be handled by the app and not you!!
		template<typename TLayer>
		void PushLayer();

		WindowHandler* GetWindowHandler();
		InputPoller* GetInputPoller();
		SceneHandler* GetSceneHandler();
		ComponentHandler* GetComponentHandler();
		BaseRenderDevice* GetRenderDevice();
		ShaderHandler* GetShaderHandler();
	};

	template<typename TLayer>
	inline void Application::PushLayer()
	{
		m_Layers.push_back(std::make_unique<TLayer>(this));
		m_Layers.back()->App = this;
	}
}