#include "pch.h"
#include "Transform.hpp"

#include <tuple>

void pge::TransformView::operator()(entt::registry& reg)
{
	auto view = reg.view<pge::CompDirty>();
	
	for (auto [entity, dirty] : view.each())
	{
		if (dirty.Modified)
		{
			DirectX::XMMATRIX model = DirectX::XMMatrixIdentity();

			auto ret = reg.try_get<pge::CompPosition, pge::CompRotation, pge::CompScale>(entity);
			
			{
				pge::CompPosition* pos = std::get<pge::CompPosition*>(ret);
				if (pos != nullptr)
					model = DirectX::XMMatrixTranslationFromVector(pos->Position);
			}

			{
				pge::CompRotation* rot = std::get<pge::CompRotation*>(ret);
				if (rot != nullptr)
					model = DirectX::XMMatrixRotationQuaternion(rot->Rotation);
			}

			{
				pge::CompScale* sca = std::get<pge::CompScale*>(ret);
				if (sca != nullptr)
					model = DirectX::XMMatrixScalingFromVector(sca->Scale);
			}

			dirty.Model = model;
			dirty.Modified = false;
		}
	}
}
