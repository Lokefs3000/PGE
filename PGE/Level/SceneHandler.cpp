#include "pch.h"
#include "SceneHandler.hpp"

pge::SceneHandler::SceneHandler()
{
	m_Registry = {};
}

pge::SceneHandler::~SceneHandler()
{
	m_Registry.clear();
}

entt::entity pge::SceneHandler::NewEntity()
{
	return m_Registry.create();
}

void pge::SceneHandler::DeleteEntity(entt::entity ent)
{
	m_Registry.destroy(ent);
}
