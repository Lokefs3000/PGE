#pragma once

#include <winerror.h>
#include <d3d12.h>

#include "RenderHardware/Common.hpp"

namespace pge
{
	class D3D12Utility
	{
	public:
		static void PrettyPrint(HRESULT hr, const char* message);

		static D3D12_FILTER ConvertFilter(TextureFilter filter);
		static D3D12_TEXTURE_ADDRESS_MODE ConvertAddressMode(AddressMode mode);
		static D3D12_SHADER_VISIBILITY ConvertVisibility(ShaderVisibility visibility);
	};
}