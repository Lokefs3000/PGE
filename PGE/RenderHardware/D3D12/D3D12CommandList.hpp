#pragma once

#include "RenderHardware/CommandList.hpp"
#include "URef.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>

namespace pge
{
	class D3D12CommandList : public CommandList
	{
	private:
		URef<ID3D12CommandAllocator> m_CommandAllocators[3];
		URef<ID3D12GraphicsCommandList> m_CommandList;

		uint32_t m_MaxFrames;
		uint32_t m_CurrentFrame;

		bool m_HasBegun;
	public:
		D3D12CommandList(ID3D12Device12* device, CommandListCreateInfo& createInfo);
		~D3D12CommandList();

		void Begin() override;
		void Close() override;

		void Submit(ID3D12CommandQueue* commandQueue);

		void SetGraphicsPipeline(GraphicsPipeline* pipeline) override;

		void SetVertexBuffer(uint8_t slot, DeviceBuffer* buffer) override;
		void SetIndexBuffer(DeviceBuffer* buffer) override;

		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset) override;
	};
}