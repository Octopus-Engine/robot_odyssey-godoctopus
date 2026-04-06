#pragma once

#include "flecs.h"
#include "octopus/utils/FixedPoint.hh"
#include "octopus/world/position/PositionContext.hh"

/// @brief Periodically checks whether any entity is within range of the owning entity.
/// Set activated to true whenever at least one entity is detected.
/// check_allies controls whether to include allied entities (true = all, false = enemies only).
struct ProximitySensor {
	octopus::Fixed range = octopus::Fixed(10);
	int64_t refresh_rate = 50;
	bool activated = false;
	bool check_allies = false;
	int64_t ticks_since_check = 0;
};

inline void declare_proximity_sensor_component(flecs::world &ecs)
{
	ecs.component<ProximitySensor>()
		.member("range", &ProximitySensor::range)
		.member("refresh_rate", &ProximitySensor::refresh_rate)
		.member("activated", &ProximitySensor::activated)
		.member("check_allies", &ProximitySensor::check_allies);
}

void declare_proximity_sensor_system(flecs::world &ecs, octopus::PositionContext &pos_context);
