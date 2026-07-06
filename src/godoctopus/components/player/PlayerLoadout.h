#pragma once

#include "flecs.h"

#include <cstdint>
#include <string>
#include <vector>

#include "octopus/utils/fast_map/fast_map.hh"

struct RuneInfo {
	// Internal name of the rune, used to identify it in the game
	// identifies the type of rune, e.g. "DamageBuffRuneRegular"
	std::string type;
	// Rune data is stored in a fast_map to allow for easy serialization and deserialization
	octopus::fast_map<std::string, int64_t> data;
};

struct PlayerRuneLoadout {
	std::vector<RuneInfo> runes;
};

struct UnitRuneSlot {
	int32_t slot_type = 0;
	bool activated = true;
	bool has_rune = false;
	RuneInfo rune;
};

struct UnitLoadout {
	std::string prefab_name;
	std::vector<UnitRuneSlot> slots;
};

struct PlayerUnitLoadout {
	std::vector<UnitLoadout> units;
};

void declare_player_loadout_component(flecs::world &ecs);
