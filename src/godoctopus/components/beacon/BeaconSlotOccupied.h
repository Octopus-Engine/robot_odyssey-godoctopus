#pragma once

#include "flecs.h"

/// @brief Placed on a beacon/spawner entity.
/// Tracks whether a spawned occupant entity is currently alive.
/// Set to true when a BeaconOccupant referencing this entity is added,
/// and back to false when that occupant is destroyed.
struct BeaconSlotOccupied {
	bool occupied = false;
};

inline void declare_beacon_slot_occupied_component(flecs::world &ecs)
{
	ecs.component<BeaconSlotOccupied>()
		.member("occupied", &BeaconSlotOccupied::occupied);
}
