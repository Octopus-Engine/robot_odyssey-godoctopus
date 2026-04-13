#pragma once

#include <string>

#include "flecs.h"
#include "octopus/utils/FixedPoint.hh"
#include "octopus/utils/Vector.hh"
#include "octopus/world/position/PositionContext.hh"

/// @brief One-shot proximity check around a world-space point.
/// Emits a CustomSignalEvent on the CustomSignalNode singleton entity when
/// at least one entity is found in range, then removes itself.
struct ProximityCustomSignal {
	octopus::Fixed range = octopus::Fixed(10);
	std::string payload = "proximity_triggered";
	uint16_t refresh_tick = 32;
	bool check_allies = false;
	bool dies_on_trigger = false;
};

inline void declare_proximity_custom_signal_component(flecs::world &ecs)
{
	ecs.component<ProximityCustomSignal>()
		.member("range", &ProximityCustomSignal::range)
		.member("payload", &ProximityCustomSignal::payload)
		.member("refresh_tick", &ProximityCustomSignal::refresh_tick)
		.member("check_allies", &ProximityCustomSignal::check_allies)
		.member("dies_on_trigger", &ProximityCustomSignal::dies_on_trigger);
}

void declare_proximity_custom_signal_system(flecs::world &ecs, octopus::PositionContext &pos_context);
