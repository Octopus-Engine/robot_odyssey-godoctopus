#pragma once

#include "godoctopus/proxy/data/InfoAvailableActionResource.h"
#include "godoctopus/proxy/data/InfoProductionQueueResource.h"
#include "godoctopus/proxy/data/InfoProxyResource.h"
#include "godoctopus/proxy/data/InfoTargetResource.h"
#include "godot_tools.h"

namespace godot {

struct InfoProxyData {
	// Info Target
	SET_GET_PARAM(octopus::Vector, position);
	SET_GET_PARAM(octopus::Vector, target);
	SET_GET_PARAM_DEF(bool, has_move_target, false);
	SET_GET_PARAM_DEF(bool, has_attack_target, false);

	// Info Production Queue
	SET_GET_PARAM(TypedArray<Ref<InfoProductionQueueResource>>, production_queue);

	// Basic info
	SET_GET_PARAM(std::string, type);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(double, hp, 0);
	SET_GET_PARAM_DEF(double, hp_max, 0);
	SET_GET_PARAM_DEF(double, armor, 0);
	SET_GET_PARAM_DEF(double, damage, 0);
	SET_GET_PARAM_DEF(double, reload_time, 0);
	SET_GET_PARAM_DEF(double, special, 0);
	SET_GET_PARAM_DEF(double, affinity, 0);
	SET_GET_PARAM_DEF(bool, proximity_sensor_activated, false);
	SET_GET_PARAM_DEF(int, pickable_id, -1);
	SET_GET_PARAM_DEF(int, rune_loads, 0);
	SET_GET_PARAM_DEF(bool, alive, true);
	SET_GET_PARAM_DEF(bool, building, true);

public:
	Ref<InfoProxyResource> duplicate() const {
		Ref<InfoProxyResource> copy = Ref<InfoProxyResource>(memnew(InfoProxyResource));
		copy->set_position({(float)get_position().x.to_double(), (float)get_position().y.to_double()});
		copy->set_target({(float)get_target().x.to_double(), (float)get_target().y.to_double()});
		copy->set_has_move_target(get_has_move_target());
		copy->set_has_attack_target(get_has_attack_target());

		copy->get_production_queue().resize(get_production_queue().size());
		for (int i = 0; i < get_production_queue().size(); ++i) {
			Ref<InfoProductionQueueResource> prod_copy = Ref<InfoProductionQueueResource>(memnew(InfoProductionQueueResource));
			Ref<InfoProductionQueueResource> prod_source = get_production_queue()[i];
			prod_copy->set_prod_name(prod_source->get_prod_name());
			prod_copy->set_progress(prod_source->get_progress());
			copy->get_production_queue()[i] = prod_copy;
		}

		copy->set_type(get_type().c_str());
		copy->set_team(get_team());
		copy->set_player(get_player());
		copy->set_hp(get_hp());
		copy->set_hp_max(get_hp_max());
		copy->set_armor(get_armor());
		copy->set_damage(get_damage());
		copy->set_reload_time(get_reload_time());
		copy->set_special(get_special());
		copy->set_affinity(get_affinity());
		copy->set_proximity_sensor_activated(get_proximity_sensor_activated());
		copy->set_pickable_id(get_pickable_id());
		copy->set_rune_loads(get_rune_loads());
		copy->set_alive(get_alive());
		copy->set_building(get_building());
		return copy;
	}

	flecs::entity entity;
};

}