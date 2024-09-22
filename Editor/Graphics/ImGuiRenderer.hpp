#pragma once

#include <memory>
#include <Resources/ResourcePtr.hpp>

struct ImDrawData;

namespace pge
{
	class Application;
	class DeviceBuffer;
	class CommandList;
	class BaseRenderDevice;
	class ShaderResource;
}

namespace ed
{
	class ImGuiRenderer
	{
	private:
		pge::BaseRenderDevice* m_RenderDevice;

		std::unique_ptr<pge::DeviceBuffer> m_VertexBuffer;
		std::unique_ptr<pge::DeviceBuffer> m_IndexBuffer;
		pge::ResourcePtr<pge::ShaderResource> m_DrawShader;

		void* m_VMap;
		void* m_IMap;
	public:
		ImGuiRenderer(pge::Application* app);
		~ImGuiRenderer();

		void Render(pge::CommandList* commandList, ImDrawData* drawData);
	};
}