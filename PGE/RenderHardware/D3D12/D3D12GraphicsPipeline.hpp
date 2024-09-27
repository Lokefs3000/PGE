#pragma once

#include "RenderHardware/GraphicsPipeline.hpp"
#include "URef.hpp"

#include <d3d12.h>

#include "D3D12DescriptorHeapMananager.hpp"

namespace pge
{
	class D3D12GraphicsPipeline : public GraphicsPipeline
	{
	private:
		D3D12DescriptorHeapManager* m_HeapManager;

		URef<ID3D12RootSignature> m_RootSignature;
		std::vector<URef<ID3D12PipelineState>> m_Pipelines;

		D3D12HeapAllocation m_Descriptors;
		pge::GraphicsPipelineCreateInfo m_CachedCreateInfo;
	public:
		D3D12GraphicsPipeline(ID3D12Device12* device, D3D12DescriptorHeapManager* heapManager, GraphicsPipelineCreateInfo& createInfo);
		~D3D12GraphicsPipeline() override;
	};
}