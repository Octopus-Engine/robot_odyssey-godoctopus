#pragma once

#include "BeaconSpawnAbility.h"
#include "core/variant/dictionary.h"

namespace godot {
class GameNode;
}

/// @brief Concrete beacon spawn ability for unit producer prefabs.
/// Uses "unit_producer" as the default prefab name when BeaconConfig is unavailable.
struct UnitProducerBeaconSpawnAbility : BeaconSpawnAbility {

	virtual std::string get_default_prefab_name() const override {
		return "unit_producer";
	}

	static std::string NAME() { return "unit_producer_beacon"; }
	virtual std::string name() const override { return UnitProducerBeaconSpawnAbility::NAME(); }
	virtual int64_t windup() const override { return 1; }
	virtual int64_t reload() const override { return 0; }
	virtual bool need_point_target() const override { return false; }
	virtual bool need_entity_target() const override { return false; }
	virtual octopus::Fixed range() const override { return octopus::Fixed(0); }
};

void declare_unit_producer_beacon_ability(flecs::world &ecs, godot::GameNode &game, Dictionary const &meta_data);
