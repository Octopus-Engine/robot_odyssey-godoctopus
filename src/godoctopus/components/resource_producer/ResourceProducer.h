#pragma once

#include <string>
#include "flecs.h"
#include "octopus/utils/FixedPoint.hh"
#include "octopus_types.h"

/// @brief Placed on an entity spawned by BeaconSpawnAbility.
/// Periodically credits resources to the owning player's ResourceStock.
struct ResourceProducer {
	std::string resource_name;
	octopus::Fixed amount = octopus::Fixed(1);
	int64_t interval = 50;
	int64_t ticks_since_last = 0;
};

inline void declare_resource_producer_component(flecs::world &ecs)
{
	ecs.component<ResourceProducer>()
		.member("resource_name", &ResourceProducer::resource_name)
		.member("amount", &ResourceProducer::amount)
		.member("interval", &ResourceProducer::interval)
		.member("ticks_since_last", &ResourceProducer::ticks_since_last);
}

void declare_resource_producer_system(flecs::world &ecs, custom_step_manager &step_manager);
