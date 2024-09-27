#include "ImGuiRenderer.hpp"

#include <imgui/imgui.h>
#include <spdlog/spdlog.h>

#include <Core/Application.hpp>
#include <RenderHardware/BaseRenderDevice.hpp>
#include <RenderHardware/CommandList.hpp>
#include <RenderHardware/DeviceBuffer.hpp>
#include <Graphics/Shaders/ShaderHandler.hpp>
#include <Resources/ShaderResource.hpp>

ed::ImGuiRenderer::ImGuiRenderer(pge::Application* app)
{
	m_RenderDevice = app->GetRenderDevice();
	
	m_DrawShader = app->GetShaderHandler()->LoadResource("Hidden/ImGui");

	m_VMap = nullptr;
	m_IMap = nullptr;
}

ed::ImGuiRenderer::~ImGuiRenderer()
{
	m_VertexBuffer->Unmap({ 0 });
	m_IndexBuffer->Unmap({ 0 });
}

void ed::ImGuiRenderer::Render(pge::CommandList* commandList, ImDrawData* drawData)
{
	if (m_VertexBuffer == nullptr || m_VertexBuffer->GetSize() < drawData->TotalVtxCount)
	{
		m_VertexBuffer.reset();
		m_VertexBuffer = m_RenderDevice->CreateDeviceBuffer({
				.Usage = pge::BufferUsage::Vertex,
				.Access = pge::BufferAccess::Write,
				.Dynamic = true,
				.Size = drawData->TotalVtxCount + (size_t)5000,
				.Stride = sizeof(ImDrawVert)
			});
		m_VMap = m_VertexBuffer->Map();

		spdlog::get("ED")->info("Resizing ImGui vertex buffer to: {}", drawData->TotalVtxCount + (size_t)5000);
	}

	if (m_IndexBuffer == nullptr || m_IndexBuffer->GetSize() < drawData->TotalIdxCount)
	{
		m_IndexBuffer.reset();
		m_IndexBuffer = m_RenderDevice->CreateDeviceBuffer({
				.Usage = pge::BufferUsage::Index,
				.Access = pge::BufferAccess::Write,
				.Dynamic = true,
				.Size = drawData->TotalIdxCount + (size_t)10000,
				.Stride = sizeof(ImDrawIdx)
			});
		m_IMap = m_IndexBuffer->Map();

		spdlog::get("ED")->info("Resizing ImGui index buffer to: {}", drawData->TotalIdxCount + (size_t)10000);
	}

	char* baseVPtr = (char*)m_VMap;
	char* baseIPtr = (char*)m_IMap;

	for (size_t i = 0; i < drawData->CmdListsCount; i++)
	{
		ImDrawList* list = drawData->CmdLists[i];
		
		memcpy(baseVPtr, list->VtxBuffer.Data, list->VtxBuffer.Size);
		memcpy(baseIPtr, list->IdxBuffer.Data, list->IdxBuffer.Size);

		baseVPtr += list->VtxBuffer.Size;
		baseIPtr += list->IdxBuffer.Size;
	}

	commandList->SetVertexBuffer(0, m_VertexBuffer.get());
	commandList->SetIndexBuffer(m_IndexBuffer.get());
}
