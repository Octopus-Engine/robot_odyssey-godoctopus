#pragma once

#include "BeaconSpawnAbility.h"
#include "core/variant/dictionary.h"

namespace godot {
class GameNode;
}

/// @brief Concrete beacon spawn ability for advanced resource producer prefabs.
/// Uses "advanced_resource_producer" as the default prefab name when BeaconConfig is unavailable.
struct AdvancedResourceProducerBeaconSpawnAbility : BeaconSpawnAbility {

	virtual std::string get_default_prefab_name() const override {
		return "advanced_resource_producer";
	}

	static std::string NAME() { return "advanced_resource_producer_beacon"; }
	virtual std::string name() const override { return AdvancedResourceProducerBeaconSpawnAbility::NAME(); }
	virtual int64_t windup() const override { return 1; }
	virtual int64_t reload() const override { return 0; }
	virtual bool need_point_target() const override { return false; }
	virtual bool need_entity_target() const override { return false; }
	virtual octopus::Fixed range() const override { return octopus::Fixed(0); }
};

/// @brief Concrete beacon spawn ability for advanced resource producer prefabs.
/// Uses "advanced_resource_producer" as the default prefab name when BeaconConfig is unavailable.
struct BigAdvancedResourceProducerBeaconSpawnAbility : BeaconSpawnAbility {

	virtual std::string get_default_prefab_name() const override {
		return "advanced_resource_producer";
	}

	bool use_sensor_activation_check() const override {
		return true;
	}

	std::string get_sensor_activation_error() const override {
			return "NEED_ALLY_NEARBY";
	}

	static std::string NAME() { return "big_advanced_resource_producer_beacon"; }
	virtual std::string name() const override { return BigAdvancedResourceProducerBeaconSpawnAbility::NAME(); }
	virtual int64_t windup() const override { return 1; }
	virtual int64_t reload() const override { return 0; }
	virtual bool need_point_target() const override { return false; }
	virtual bool need_entity_target() const override { return false; }
	virtual octopus::Fixed range() const override { return octopus::Fixed(0); }
};

void declare_advanced_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &game, Dictionary const &meta_data);
