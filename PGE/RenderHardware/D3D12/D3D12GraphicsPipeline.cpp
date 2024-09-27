#include "pch.h"
#include "D3D12GraphicsPipeline.hpp"

#include "D3D12Utility.hpp"

#include <d3dx12.h>

pge::D3D12GraphicsPipeline::D3D12GraphicsPipeline(ID3D12Device12* device, D3D12DescriptorHeapManager* heapManager, GraphicsPipelineCreateInfo& createInfo)
{
	{
		D3D12_ROOT_SIGNATURE_DESC1 desc{};
		desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		std::vector<D3D12_ROOT_PARAMETER> params{};
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers{};

		for (ShaderValueParameter& param : createInfo.Parameters)
		{
			switch (param.Id)
			{
			case pge::ShaderValueId::ConstantBuffer:
			{
				break;
			}
			case pge::ShaderValueId::SamplerState:
			{
				ShaderValueParameter::SamplerStateData& data = std::get<ShaderValueParameter::SamplerStateData>(param.Variants);

				D3D12_STATIC_SAMPLER_DESC samplerDesc{};
				samplerDesc.Filter = D3D12Utility::ConvertFilter(data.Filter);
				samplerDesc.AddressU = D3D12Utility::ConvertAddressMode(data.AddressU);
				samplerDesc.AddressV = D3D12Utility::ConvertAddressMode(data.AddressV);
				samplerDesc.ShaderRegister = param.Index;
				samplerDesc.ShaderVisibility = D3D12Utility::ConvertVisibility(param.Visbility);

				samplers.push_back(samplerDesc);
				break;
			}
			case pge::ShaderValueId::Texture:
			{
				break;
			}
			default: break;
			}
		}

		heapManager->DataAllocate(m_Descriptors, desc.NumParameters);
	}

	m_HeapManager = heapManager;
	m_CachedCreateInfo = std::move(createInfo);
}

pge::D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
{
	m_HeapManager->DataFree(m_Descriptors);
}
