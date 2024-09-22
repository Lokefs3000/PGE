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
}