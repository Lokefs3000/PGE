#pragma once

#include "ComponentViewBase.h"

#include <DirectXMath.h>

namespace pge
{
	struct TransformView : public ComponentViewBase
	{
		void operator()(entt::registry& reg);
	};

	struct CompPosition
	{
		DirectX::XMVECTOR Position;
	};

	struct CompRotation
	{
		DirectX::XMVECTOR Rotation; //quat
	};

	struct CompScale
	{
		DirectX::XMVECTOR Scale;
	};

	struct CompDirty
	{
		DirectX::XMMATRIX Model;
		bool Modified;
	};
}