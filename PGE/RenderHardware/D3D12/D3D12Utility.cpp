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
