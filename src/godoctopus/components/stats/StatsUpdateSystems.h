#pragma once

#include "flecs.h"
#include "StatsSet.h"

namespace godoctopus {

struct BaseStats {
	StatsSet stats;
};

struct CurrentStats {
	StatsSet stats;
};

void declare_stateupdate_systems(flecs::world &ecs);

} // namespace godoctopus
