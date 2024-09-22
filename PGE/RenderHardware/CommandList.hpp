#pragma once

#include <cstdint>

namespace pge
{
	struct CommandListCreateInfo
	{
		uint32_t MaxFramesInFlight;
	};

	class GraphicsPipeline;
	class DeviceBuffer;
	class Framebuffer;

	class CommandList
	{
	private:

	public:
		CommandList();
		~CommandList();

		virtual void Begin() = 0;
		virtual void Close() = 0;

		virtual void SetGraphicsPipeline(GraphicsPipeline* pipeline) = 0;

		virtual void SetVertexBuffer(uint8_t slot, DeviceBuffer* buffer) = 0;
		virtual void SetIndexBuffer(DeviceBuffer* buffer) = 0;
		
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset) = 0;
	};
}