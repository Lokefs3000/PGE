#pragma once

#include "RenderHardware/GraphicsPipeline.hpp"
#include "URef.hpp"

#include <d3d12.h>

namespace pge
{
	class D3D12GraphicsPipeline : public GraphicsPipeline
	{
	private:
		URef<ID3D12RootSignature> m_RootSignature;
		URef<ID3D12PipelineState> m_Pipeline;
	public:
		D3D12GraphicsPipeline(ID3D12Device12* device, GraphicsPipelineCreateInfo& createInfo);
		~D3D12GraphicsPipeline() override;
	};
}