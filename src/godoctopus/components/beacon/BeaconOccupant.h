#pragma once

#include "flecs.h"

/// @brief Placed on a spawned occupant entity (e.g. a resource producer).
/// Holds a reference to the beacon/spawner entity that carries BeaconSlotOccupied.
/// Observers on this component keep BeaconSlotOccupied.occupied in sync:
///   OnSet  → occupied = true
///   OnRemove → occupied = false
struct BeaconOccupant {
	flecs::entity slot_entity;
};

void declare_beacon_occupant_component(flecs::world &ecs);
void declare_beacon_occupant_observers(flecs::world &ecs);
