#pragma once

#include "BeaconSpawnAbility.h"

namespace godot {
class GameNode;
}

/// @brief Concrete beacon spawn ability for basic resource producer prefabs.
/// Uses "basic_resource_producer" as the default prefab name when BeaconConfig is unavailable.
struct BasicResourceProducerBeaconSpawnAbility : BeaconSpawnAbility {

	virtual std::string get_default_prefab_name() const override {
		return "basic_resource_producer";
	}

	static std::string NAME() { return "basic_resource_producer_beacon"; }
	virtual std::string name() const override { return BasicResourceProducerBeaconSpawnAbility::NAME(); }
	virtual int64_t windup() const override { return 1; }
	virtual int64_t reload() const override { return 0; }
	virtual bool need_point_target() const override { return false; }
	virtual bool need_entity_target() const override { return false; }
	virtual octopus::Fixed range() const override { return octopus::Fixed(0); }
};

void declare_basic_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &game);
