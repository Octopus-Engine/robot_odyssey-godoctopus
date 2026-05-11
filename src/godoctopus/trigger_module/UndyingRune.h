#pragma once

#include "flecs.h"

#include "octopus_types.h"

namespace godot {
	class SmartMMeshLibrary;
}

struct ApplyUndyingBuffOnRuneLoad { int32_t level = 0; };

void declare_undying_rune_triggers(flecs::world &ecs, custom_step_manager& manager, godot::SmartMMeshLibrary *library = nullptr);
