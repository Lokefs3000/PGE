#include "pch.h"
#include "ComponentHandler.hpp"

#include "ComponentViewBase.h"
#include "Level/SceneHandler.hpp"

pge::ComponentHandler::ComponentHandler()
{
	m_RefSceneHandler = nullptr;
}

pge::ComponentHandler::~ComponentHandler()
{
	m_Views.clear();

	m_RefSceneHandler = nullptr;
}

void pge::ComponentHandler::Dispatch()
{
	for (ComponentViewBase& view : m_Views)
	{
		view(m_RefSceneHandler->m_Registry);
	}
}

void pge::ComponentHandler::Connect(SceneHandler* sceneHandler)
{
	m_RefSceneHandler = sceneHandler;
}
