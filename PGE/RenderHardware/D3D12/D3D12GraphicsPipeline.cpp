#include "pch.h"
#include "D3D12GraphicsPipeline.hpp"

#include "D3D12Utility.hpp"

#include <d3dx12.h>

pge::D3D12GraphicsPipeline::D3D12GraphicsPipeline(ID3D12Device12* device, GraphicsPipelineCreateInfo& createInfo)
{
	{
		
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = m_RootSignature.Get();

		desc.VS = { createInfo.VertexShader.data(), createInfo.VertexShader.size() };
		desc.PS = { createInfo.PixelShader.data(), createInfo.PixelShader.size() };

		desc.BlendState.AlphaToCoverageEnable = createInfo.Blend.AlphaToCoverage;
		desc.BlendState.IndependentBlendEnable = createInfo.Blend.IndependentBlendEnable;
		for (size_t i = 0; i < 8; i++)
		{
			if (!createInfo.Blend.RenderTarget[i].BlendEnable) break;
			RenderTargetBlendDescription& rtCreateInfo = createInfo.Blend.RenderTarget[i];

			D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = desc.BlendState.RenderTarget[i];
			blendDesc.BlendEnable = rtCreateInfo.BlendEnable;
			blendDesc.LogicOpEnable = rtCreateInfo.LogicOpEnable;
			blendDesc.SrcBlend = (D3D12_BLEND)rtCreateInfo.SrcBlend;
			blendDesc.DestBlend = (D3D12_BLEND)rtCreateInfo.DestBlend;
			blendDesc.BlendOp = (D3D12_BLEND_OP)rtCreateInfo.BlendOp;
			blendDesc.SrcBlendAlpha = (D3D12_BLEND)rtCreateInfo.SrcBlendAlpha;
			blendDesc.DestBlendAlpha = (D3D12_BLEND)rtCreateInfo.DestBlendAlpha;
			blendDesc.BlendOpAlpha = (D3D12_BLEND_OP)rtCreateInfo.BlendOpAlpha;
			blendDesc.LogicOp = (D3D12_LOGIC_OP)rtCreateInfo.LogicOp;
			blendDesc.RenderTargetWriteMask = rtCreateInfo.RenderTargetWriteMask;
		}

		desc.RasterizerState.FillMode = (D3D12_FILL_MODE)createInfo.Rasterizer.FillMode;
		desc.RasterizerState.CullMode = (D3D12_CULL_MODE)createInfo.Rasterizer.FillMode;
		desc.RasterizerState.FrontCounterClockwise = createInfo.Rasterizer.FrontCounterClockwise;
		desc.RasterizerState.DepthBias = createInfo.Rasterizer.DepthBias;
		desc.RasterizerState.DepthBiasClamp = createInfo.Rasterizer.DepthBiasClamp;
		desc.RasterizerState.SlopeScaledDepthBias = createInfo.Rasterizer.SlopeScaledDepthBias;
		desc.RasterizerState.DepthClipEnable = createInfo.Rasterizer.DepthClipEnable;
		desc.RasterizerState.MultisampleEnable = createInfo.Rasterizer.MultisampleEnable;
		desc.RasterizerState.AntialiasedLineEnable = createInfo.Rasterizer.AntialiasedLineEnable;
		desc.RasterizerState.ForcedSampleCount = createInfo.Rasterizer.ForcedSampleCount;

		desc.DepthStencilState.DepthEnable = createInfo.DepthStencil.DepthEnable;
		desc.DepthStencilState.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)createInfo.DepthStencil.DepthWriteMask;
		desc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)createInfo.DepthStencil.DepthFunc;
		desc.DepthStencilState.StencilEnable = createInfo.DepthStencil.StencilEnable;
		desc.DepthStencilState.StencilReadMask = createInfo.DepthStencil.StencilReadMask;
		desc.DepthStencilState.StencilWriteMask = createInfo.DepthStencil.StencilWriteMask;
		desc.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)createInfo.DepthStencil.FrontFace.StencilFailOp;
		desc.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)createInfo.DepthStencil.FrontFace.StencilDepthFailOp;
		desc.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)createInfo.DepthStencil.FrontFace.StencilPassOp;
		desc.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)createInfo.DepthStencil.FrontFace.StencilFunc;
		desc.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)createInfo.DepthStencil.BackFace.StencilFailOp;
		desc.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)createInfo.DepthStencil.BackFace.StencilDepthFailOp;
		desc.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)createInfo.DepthStencil.BackFace.StencilPassOp;
		desc.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)createInfo.DepthStencil.BackFace.StencilFunc;

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputs;
		inputs.resize(createInfo.InputLayout.InputElementDescs.size());
		for (size_t i = 0; i < inputs.size(); i++)
		{
			InputElementDescription& elemCreateInfo = createInfo.InputLayout.InputElementDescs[i];

			D3D12_INPUT_ELEMENT_DESC& elemDesc = inputs[i];
			elemDesc.SemanticName = elemCreateInfo.SemanticName;
			elemDesc.SemanticIndex = elemCreateInfo.SemanticIndex;
			elemDesc.Format = (DXGI_FORMAT)elemCreateInfo.Format;
			elemDesc.InputSlot = elemCreateInfo.InputSlot;
			elemDesc.AlignedByteOffset = elemCreateInfo.AlignedByteOffset;
			elemDesc.InputSlotClass = (D3D12_INPUT_CLASSIFICATION)elemCreateInfo.InputSlotClass;
			elemDesc.InstanceDataStepRate = elemCreateInfo.InstanceDataStepRate;
		}
		desc.InputLayout.pInputElementDescs = inputs.data();
		desc.InputLayout.NumElements = inputs.size();

		desc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)createInfo.PrimitiveTopologyType;

		while (desc.NumRenderTargets < 8 && createInfo.RTVFormats[desc.NumRenderTargets++] != Format::DXGI_FORMAT_UNKNOWN);
		desc.NumRenderTargets--;
		memcpy(&desc.RTVFormats, &desc.RTVFormats, sizeof(desc.RTVFormats));
		desc.DSVFormat = (DXGI_FORMAT)createInfo.DSVFormat;

		HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_Pipeline.GetAddressOf()));
		if (FAILED(hr))
		{
			if (m_RootSignature.Get() != nullptr) m_RootSignature->Release();
			
			D3D12Utility::PrettyPrint(hr, "Failed to create graphics pipeline state!");
			return;
		}
	}
}

pge::D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
{
}
