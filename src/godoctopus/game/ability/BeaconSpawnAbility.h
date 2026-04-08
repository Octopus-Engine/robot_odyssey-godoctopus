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

namespace godot {
class GameNode;
}

struct BeaconSpawnAbility : octopus::AbilityTemplate<custom_step_manager> {

	virtual octopus::UpgradeRequirement get_requirements() const { return {}; }
	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const { return {}; }

	virtual std::string is_castable(flecs::entity caster, flecs::world const &) const override {
		ProximitySensor const *sensor = caster.try_get<ProximitySensor>();
		if (!sensor || !sensor->activated) {
			return "ALLY_PROXIMITY_SENSOR_NOT_ACTIVATED";
		}
		BeaconSlotOccupied const *slot = caster.try_get<BeaconSlotOccupied>();
		if (slot && slot->occupied) {
			return "BEACON_SLOT_OCCUPIED";
		}
		return "";
	}

	virtual void cast(flecs::entity caster, octopus::Vector, flecs::entity, flecs::world const &ecs, custom_step_manager &) const {
		octopus::Logger::getDebug() <<"Beacon spawn :: Entering"<<std::endl;
		ProximitySensor const *sensor = caster.try_get<ProximitySensor>();
		if (!sensor || !sensor->activated) {
			return;
		}

		BeaconConfig const *config = caster.try_get<BeaconConfig>();
		if (!config) {
			return;
		}

		octopus::Position const *pos = caster.try_get<octopus::Position>();
		if (!pos) {
			return;
		}
		octopus::Logger::getDebug() <<"Beacon spawn :: ability cast"<<std::endl;

		octopus::PlayerAppartenance const *appartenance = caster.try_get<octopus::PlayerAppartenance>();
		uint32_t player_idx = appartenance ? appartenance->idx : 0;
		octopus::Team const *team = caster.try_get<octopus::Team>();
		uint16_t player_team = team ? team->team : 0;

		octopus::Position spawn_pos = *pos;
		std::string producer_prefab_name = config->producer_prefab_name;

		octopus::EntityCreationStep step;
		step.set_up_function = [spawn_pos, player_idx, player_team, producer_prefab_name, caster](flecs::entity new_ent, flecs::world const &world_p) {
			auto e = new_ent
				.set<octopus::Position>(spawn_pos)
				.set<octopus::Team>({player_team})
				.set<octopus::PlayerAppartenance>({player_idx})
				.set<BeaconOccupant>({caster});
			if (!producer_prefab_name.empty()) {
				e.is_a(world_p.prefab(producer_prefab_name.c_str()));
			}
		};

		// StepEntityManager is always mutably accessible — cast away const on the world wrapper
		const_cast<flecs::world &>(ecs).try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step);
	}

	static std::string NAME() { return "beacon_spawn"; }
	virtual std::string name() const { return BeaconSpawnAbility::NAME(); }
	virtual int64_t windup() const { return 1; }
	virtual int64_t reload() const { return 0; }
	virtual bool need_point_target() const { return false; }
	virtual bool need_entity_target() const { return false; }
	virtual octopus::Fixed range() const { return octopus::Fixed(0); }
};

void declare_beacon_spawn_ability(flecs::world &ecs, godot::GameNode &game);
