#pragma once

#include <entt/fwd.hpp>

namespace pge
{
	struct ComponentViewBase
	{
		virtual void operator()(entt::registry& reg) = 0;
	};
}