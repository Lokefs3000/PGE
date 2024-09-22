#pragma once

#include <entt/entt.hpp>

namespace pge
{
	class SceneHandler
	{
	private:
		entt::registry m_Registry;

		friend class ComponentHandler;
	public:
		SceneHandler();
		~SceneHandler();

		entt::entity NewEntity();
		void DeleteEntity(entt::entity ent);

		template<typename T>
		T& AddComponent(entt::entity& ent);
		template<typename T>
		void RemoveComponent(entt::entity& ent);
	};

	template<typename T>
	inline T& SceneHandler::AddComponent(entt::entity& ent)
	{
		m_Registry.emplace(ent, T());
		return m_Registry.get<T>(ent);
	}

	template<typename T>
	inline void SceneHandler::RemoveComponent(entt::entity& ent)
	{
		m_Registry.remove<T>(ent);
	}
}