#pragma once

#include "CircularSpawnAbility.h"

namespace godot {
class GameNode;
}

/// @brief Concrete circular spawn ability for building slot prefabs.
/// Spawns 4 building slots in a circle with radius 10 around the caster.
struct BuildingSlotSpawnAbility10_4 : CircularSpawnAbility {
	virtual octopus::Fixed get_spawn_radius() const override { return octopus::Fixed(10); }
	virtual uint32_t get_spawn_count() const override { return 4; }
	virtual std::string get_prefab_name() const override { return "building_slot"; }
	static std::string NAME() { return "building_slot_spawn_10_4"; }
	virtual std::string name() const override { return BuildingSlotSpawnAbility10_4::NAME(); }
};

/// @brief Concrete circular spawn ability for building slot prefabs.
/// Spawns 8 building slots in a circle with radius 17 around the caster.
struct BuildingSlotSpawnAbility17_8 : CircularSpawnAbility {
	virtual octopus::Fixed get_spawn_radius() const override { return octopus::Fixed(17); }
	virtual uint32_t get_spawn_count() const override { return 8; }
	virtual std::string get_prefab_name() const override { return "building_slot"; }
	static std::string NAME() { return "building_slot_spawn_17_8"; }
	virtual std::string name() const override { return BuildingSlotSpawnAbility17_8::NAME(); }
};

/// @brief Concrete circular spawn ability for building slot prefabs.
/// Spawns 16 building slots in a circle with radius 24 around the caster.
struct BuildingSlotSpawnAbility24_16 : CircularSpawnAbility {
	virtual octopus::Fixed get_spawn_radius() const override { return octopus::Fixed(24); }
	virtual uint32_t get_spawn_count() const override { return 16; }
	virtual std::string get_prefab_name() const override { return "building_slot"; }
	static std::string NAME() { return "building_slot_spawn_24_16"; }
	virtual std::string name() const override { return BuildingSlotSpawnAbility24_16::NAME(); }
};

void declare_building_slot_spawn_ability(flecs::world &ecs, godot::GameNode &game);
