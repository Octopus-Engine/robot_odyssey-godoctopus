#pragma once

#include <string>
#include "flecs.h"
#include "octopus/utils/FixedPoint.hh"

/// @brief Configuration for a beacon building.
/// Stores the resource production parameters consumed by BeaconSpawnAbility at cast time.
struct BeaconConfig {
	std::string producer_prefab_name;
};

inline void declare_beacon_config_component(flecs::world &ecs)
{
	ecs.component<BeaconConfig>()
		.member("producer_prefab_name", &BeaconConfig::producer_prefab_name);
}
