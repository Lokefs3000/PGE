#pragma once

#include <vector>
#include <variant>

#include "Common.hpp"

namespace pge
{
	struct RenderTargetBlendDescription
	{
		bool BlendEnable;
		bool LogicOpEnable;

		Blend SrcBlend;
		Blend DestBlend;
		BlendOperation BlendOp;

		Blend SrcBlendAlpha;
		Blend DestBlendAlpha;
		BlendOperation BlendOpAlpha;

		LogicOperation LogicOp;

		uint8_t RenderTargetWriteMask;
	};

	struct BlendDescription
	{
		bool AlphaToCoverage;
		bool IndependentBlendEnable;

		RenderTargetBlendDescription RenderTarget[8];
	};

	struct RasterizerDescription
	{
		FillMode FillMode;
		CullMode CullMode;
	};

	struct DepthStencilOpDescription
	{
		DepthStencilOperation StencilFailOp;
		DepthStencilOperation StencilDepthFailOp;
		DepthStencilOperation StencilPassOp;
		ComparisonFunction StencilFunc;
	};

	struct DepthStencilDescription
	{
		bool DepthEnable;

		DepthWriteMask DepthWriteMask;
		ComparisonFunction DepthFunc;

		bool StencilEnable;

		uint8_t StencilReadMask;
		uint8_t StencilWriteMask;

		DepthStencilOpDescription FrontFace;
		DepthStencilOpDescription BackFace;
	};

	struct InputElementDescription
	{
		std::string_view SemanticName;
		uint8_t SemanticIndex;
		ParameterSize Size;
	};

	struct ShaderLocalVariable
	{
		std::string Name;
		ParameterSize Size;
		SemanticId Semantic;
		uint8_t Slot;
	};

	struct ShaderValueParameter
	{
		struct TextureData
		{
			pge::TextureDimension Dimension;
		};

		struct SamplerStateData
		{
			bool IsStatic;

			pge::TextureFilter Filter;
			pge::AddressMode AddressU;
			pge::AddressMode AddressV;
		};

		struct ConstantBufferData
		{
			bool IsStructured;
			std::vector<ShaderLocalVariable> Variables;
		};

		std::string Name;
		uint16_t Index;

		ShaderValueId Id;
		ShaderVisibility Visbility;

		std::variant<TextureData, SamplerStateData, ConstantBufferData> Variants;
	};

	struct GraphicsPipelineCreateInfo
	{
		std::vector<char> VertexShader;
		std::vector<char> PixelShader;

		RenderTargetBlendDescription Blend;
		RasterizerDescription Rasterizer;
		DepthStencilDescription DepthStencil;
		std::vector<InputElementDescription> InputLayout;
		std::vector<ShaderValueParameter> Parameters;

		PrimitiveTopology PrimitiveTopologyType;
	};

	class GraphicsPipeline
	{
	private:

	public:
		GraphicsPipeline();
		virtual ~GraphicsPipeline();
	};
}