#pragma once

#include <memory>

#include "Core/ApplicationLayer.hpp"

namespace pge
{
	class CommandList;
}

namespace ed
{
	class ImGuiRenderer;

	class EditorLayer : public pge::ApplicationLayer
	{
	private:
		std::unique_ptr<pge::CommandList> m_CommandList;
		std::unique_ptr<ImGuiRenderer> m_ImGuiRenderer;
	public:
		EditorLayer(pge::Application* application);
		~EditorLayer() override;

		void Setup() override;
		void Update() override;
	};
}