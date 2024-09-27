#include "pch.h"
#include "D3D12Utility.hpp"

#include <spdlog/spdlog.h>

void pge::D3D12Utility::PrettyPrint(HRESULT hr, const char* message)
{
	if (FAILED(hr))
	{
		spdlog::get("PGE")->error(message);
		spdlog::get("PGE")->error("Code: {:#06x}", hr);
	}
}

D3D12_FILTER pge::D3D12Utility::ConvertFilter(TextureFilter filter)
{
	return D3D12_FILTER();
}

D3D12_TEXTURE_ADDRESS_MODE pge::D3D12Utility::ConvertAddressMode(AddressMode mode)
{
	return D3D12_TEXTURE_ADDRESS_MODE();
}

D3D12_SHADER_VISIBILITY pge::D3D12Utility::ConvertVisibility(ShaderVisibility visibility)
{
	return D3D12_SHADER_VISIBILITY();
}
