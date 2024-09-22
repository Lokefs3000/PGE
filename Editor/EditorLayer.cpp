#include "EditorLayer.hpp"

#include <imgui/imgui.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Core/Application.hpp>
#include <RenderHardware/BaseRenderDevice.hpp>
#include <RenderHardware/CommandList.hpp>

#include <Graphics/Shaders/ShaderHandler.hpp>

#include "Resources/ODShaderPackage.hpp"

#include "Graphics/ImGuiRenderer.hpp"

static std::shared_ptr<spdlog::logger> m_Logger;

ed::EditorLayer::EditorLayer(pge::Application* application)
{
	m_Logger = spdlog::stdout_color_mt("ED");

	{
		std::unique_ptr<pge::ShaderPackage> package = std::make_unique<ODShaderPackage>("Shaders/", "LocalCache/Shaders/");
		application->GetShaderHandler()->AddPackage(package);
	}

	m_CommandList = application->GetRenderDevice()->CreateCommandList({ .MaxFramesInFlight = 2 });

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = { 1336, 726 };
	unsigned char* pix; int w, h;
	io.Fonts->GetTexDataAsRGBA32(&pix, &w, &h);
	io.Fonts->SetTexID((ImTextureID)1);
}

ed::EditorLayer::~EditorLayer()
{
	ImGui::DestroyContext();

	m_Logger.reset();
}

void ed::EditorLayer::Setup()
{
	m_ImGuiRenderer = std::make_unique<ImGuiRenderer>(App);
}

void ed::EditorLayer::Update()
{
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	m_ImGuiRenderer->Render(m_CommandList.get(), ImGui::GetDrawData());

	App->GetRenderDevice()->SubmitCommandList(m_CommandList.get());
}
