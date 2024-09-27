#pragma once

namespace pge
{
	enum class TextureFilter : uint8_t
	{
		Unkown,
		Linear,
		Point,
	};

	enum class AddressMode : uint8_t
	{
		Unkown,
		Wrap,
		Repeat,
		Mirror
	};

	//in bytes
	enum class ParameterSize : uint8_t
	{
		Byte1 = 1,
		Byte2 = 2,
		Byte3 = 3,
		Byte4 = 4,

		Float1 = 4,
		Float2 = 8,
		Float3 = 12,
		Float4 = 16,

		Double1 = 8,
		Double2 = 16,
		Double3 = 24,
		Double4 = 32
	};

	//t = typeless, f = float, u = unsigned, s = signed, un = unorm, sn = norm
	enum class TextureFormat : uint8_t
	{
		Unkown,

		RGBA32t,
		RGBA32f,
		RGBA32u,
		RGBA32s,

		RGB32t,
		RGB32f,
		RGB32u,
		RGB32s,

		RGBA16t,
		RGBA16f,
		RGBA16u,
		RGBA16s,

		RG32t,
		RG32f,
		RG32u,
		RG32s,

		RGBA8t,
		RGBA8un,
		RGBA8un_sRGB,
		RGBA8u,
		RGBA8sn,
		RGBA8s,

		RG16t,
		RG16f,
		RG16un,
		RG16u,
		RG16sn,
		RG16i,

		R32t,
		D32f,
		R32f,
		R32u,
		R32s,

		D24un_S8u,
		R24un_X8u,

		RG8t,
		RG8un,
		RG8u,
		RG8sn,
		RG8s,

		R16t,
		R16f,
		D16un,
		R16un,
		R16u,
		R16sn,
		RG16s,
		
		R8t,
		R8un,
		R8u,
		R8sn,
		R8s,

		BC1t,
		BC1un,
		BC1un_sRGB,

		BC2t,
		BC2un,
		BC2un_sRGB,

		BC3t,
		BC3un,
		BC3un_sRGB,

		BC4t,
		BC4un,
		BC4sn,

		BC5t,
		BC5un,
		BC5sn,

		BGRA8t,
		BGRA8un_sRGB,

		BC7t,
		BC7un,
		BC7un_sRGB,
	};

	enum class Blend : uint8_t
	{
		Zero,
		One,
		SourceColor,
		InverseSourceColor,
		SourceAlpha,
		InverseSourceAlpha,
		DestinationAlpha,
		InverseDestinationAlpha,
		DestinationColor,
		InverseDestinationColor,
		Source1Color,
		InverseSource1Color,
		Source1Alpha,
		InverseSource1Alpha
	};

	enum class BlendOperation : uint8_t
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class LogicOperation : uint8_t
	{
		Clear,
		Set,
		Copy,
		CopyInverted,
		Noop,
		Invert,
		And,
		Nand,
		Or,
		Nor,
		Xor,
		Equiv,
		AndReverse,
		AndInverted,
		OrReverse,
		OrInverted
	};

	enum class FillMode : uint8_t
	{
		Wireframe,
		Solid
	};

	enum class CullMode : uint8_t
	{
		None,
		Front,
		Back
	};

	enum class DepthWriteMask : uint8_t
	{
		Zero,
		All
	};

	enum class ComparisonFunction : uint8_t
	{
		None,
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};

	enum class DepthStencilOperation : uint8_t
	{
		Keep,
		Zero,
		Replace,
		Invert,
		Increment,
		Decrement
	};

	enum class PrimitiveTopology : uint8_t
	{
		Undefined,
		Point,
		Line,
		Triangle
	};

	enum class ShaderValueId : uint8_t
	{
		ConstantBuffer,
		Texture,
		SamplerState
	};

	enum class ShaderVisibility : uint8_t
	{
		All,
		VertexOnly,
		PixelOnly
	};

	enum class TextureDimension : uint8_t
	{
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture3D,
		Texture3DArray,
	};

	enum class SemanticId : uint8_t
	{
		Unkown,
		Position,
		Texcoord,
		Normal,
		Bitangent,
		Tangent,
		Color
	};
}