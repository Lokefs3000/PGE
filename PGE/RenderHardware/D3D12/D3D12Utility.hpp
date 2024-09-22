#pragma once

#include <winerror.h>

namespace pge
{
	class D3D12Utility
	{
	public:
		static void PrettyPrint(HRESULT hr, const char* message);
	};
}