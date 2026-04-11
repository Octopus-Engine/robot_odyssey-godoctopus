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
	octopus::Vector pos;
	octopus::Fixed range = octopus::Fixed(10);
	std::string payload = "proximity_triggered";
	int tree_idx = 0;
};

inline void declare_proximity_custom_signal_component(flecs::world &ecs)
{
	ecs.component<ProximityCustomSignal>()
		.member("range", &ProximityCustomSignal::range)
		.member("payload", &ProximityCustomSignal::payload)
		.member("tree_idx", &ProximityCustomSignal::tree_idx);
}

void declare_proximity_custom_signal_system(flecs::world &ecs, octopus::PositionContext &pos_context);
