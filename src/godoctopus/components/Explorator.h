#pragma once

#include "flecs.h"

struct Explorator {
	int32_t visibility_range = 30;
};

inline void declare_explorator_component(flecs::world &ecs)
{
	ecs.component<Explorator>()
		.member("visibility_range", &Explorator::visibility_range);
}
