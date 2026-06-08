#pragma once

#include "flecs.h"

#include <cstdint>
#include <string>
#include <vector>

struct PlayerRuneEntry
{
	std::string rune_internal_name;
	std::string rune_resource_path;
	int64_t level = 1;
};

struct PlayerRuneInventory
{
	std::vector<PlayerRuneEntry> runes;
};

struct PlayerRuneSlotData
{
	int32_t slot_type = 0;
	bool locked = false;
	bool has_rune = false;
	std::string rune_internal_name;
	std::string rune_resource_path;
	int64_t rune_level = 1;
};

struct PlayerUnitLoadoutEntry
{
	std::string prefab_name;
	std::vector<PlayerRuneSlotData> slots;
};

struct PlayerUnitLoadout
{
	std::vector<PlayerUnitLoadoutEntry> units;
};

void declare_player_loadout_component(flecs::world &ecs);
