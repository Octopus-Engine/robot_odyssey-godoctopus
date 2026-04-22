#pragma once

#include "flecs.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/world/ability/AbilityTemplateLibrary.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "octopus/utils/log/Logger.hh"

#include "octopus_types.h"
#include "godoctopus/components/proximity_sensor/ProximitySensor.h"
#include "godoctopus/components/beacon/BeaconConfig.h"
#include "godoctopus/components/beacon/BeaconSlotOccupied.h"
#include "godoctopus/components/beacon/BeaconOccupant.h"
#include "godoctopus/components/resource_producer/ResourceProducer.h"
#include "BeaconSpawnAbility.h"

namespace godot {
class GameNode;
}

/// @brief Concrete beacon spawn ability that requires an activated ProximitySensor.
/// This is the original beacon spawn behavior—checks for proximity requirement in is_castable().
/// Demonstrates how subclasses can implement their own castability logic via is_castable() override.
struct ProximityBeaconSpawnAbility : BeaconSpawnAbility {

	virtual std::string is_castable(flecs::entity caster, flecs::world const &ecs) const override {
		// Check proximity sensor requirement first
		ProximitySensor const *sensor = caster.try_get<ProximitySensor>();
		if (!sensor || !sensor->activated) {
			return "ALLY_PROXIMITY_SENSOR_NOT_ACTIVATED";
		}

		// Then call parent to check common requirements (BeaconSlotOccupied, additional errors)
		return BeaconSpawnAbility::is_castable(caster, ecs);
	}

	static std::string NAME() { return "proximity_beacon_spawn"; }
	virtual std::string name() const override { return ProximityBeaconSpawnAbility::NAME(); }
	virtual int64_t windup() const override { return 1; }
	virtual int64_t reload() const override { return 0; }
	virtual bool need_point_target() const override { return false; }
	virtual bool need_entity_target() const override { return false; }
	virtual octopus::Fixed range() const override { return octopus::Fixed(0); }
};

void declare_proximity_beacon_ability(flecs::world &ecs, godot::GameNode &game);
