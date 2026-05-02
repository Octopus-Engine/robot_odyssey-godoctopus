#pragma once

#include "flecs.h"

#include "octopus_types.h"

// spawn a clone unit when it dies
struct SpawnCloneUnitRune { int32_t level = 0; };
// spawn a small unit when 5 runes are loaded
struct SpawnNewUnitRune { int32_t level = 0; };

void declare_spawn_unit_triggers(flecs::world &ecs, octopus::PositionContext const &ctx);
