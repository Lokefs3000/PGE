#pragma once

#include <vector>

namespace pge
{
	struct ComponentViewBase;
	class SceneHandler;

	class ComponentHandler
	{
	private:
		std::vector<ComponentViewBase> m_Views;

		SceneHandler* m_RefSceneHandler;
	public:
		ComponentHandler();
		~ComponentHandler();

		void Dispatch();

		template<typename TView>
		void RegisterView();

		void Connect(SceneHandler* sceneHandler);
	};

	template<typename TView>
	inline void ComponentHandler::RegisterView()
	{
		m_Views.push_back(TView{});
	}
}