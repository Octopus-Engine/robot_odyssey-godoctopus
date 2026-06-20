#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class InfoTargetResource : public Resource {
	GDCLASS(InfoTargetResource, Resource)

	SET_GET_PARAM_DEF(Vector2, position, Vector2());
	SET_GET_PARAM_DEF(bool, is_attack, false);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoTargetResource, VECTOR2, position);
		ADD_SIMPLE_PROP(InfoTargetResource, BOOL, is_attack);
	}
};

class InfoProductionQueueResource : public Resource {
	GDCLASS(InfoProductionQueueResource, Resource)

	SET_GET_PARAM_DEF(String, prod_name, "");
	SET_GET_PARAM_DEF(double, progress, 0);
	SET_GET_PARAM_DEF(int, queue_idx, 0);
	SET_GET_PARAM_DEF(int64_t, source_entity_id, -1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoProductionQueueResource, STRING, prod_name);
		ADD_SIMPLE_PROP(InfoProductionQueueResource, FLOAT, progress);
		ADD_SIMPLE_PROP(InfoProductionQueueResource, INT, queue_idx);
		ADD_SIMPLE_PROP(InfoProductionQueueResource, INT, source_entity_id);
	}
};

class InfoAvailableActionResource : public Resource {
	GDCLASS(InfoAvailableActionResource, Resource)
public:
	enum ActionType {
		ACTION_CAST = 0,
		ACTION_PRODUCTION = 1,
	};

private:
	SET_GET_PARAM_DEF(String, action_name, "");
	SET_GET_PARAM_DEF(int, action_type, ACTION_CAST);
	SET_GET_PARAM_DEF(int64_t, source_entity_id, -1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoAvailableActionResource, STRING, action_name);
		ADD_SIMPLE_PROP(InfoAvailableActionResource, INT, action_type);
		ADD_SIMPLE_PROP(InfoAvailableActionResource, INT, source_entity_id);
	}
};

class InfoProxyResource : public Resource {
	GDCLASS(InfoProxyResource, Resource)

	// Info Target
	SET_GET_PARAM_DEF(Vector2, position, Vector2());
	SET_GET_PARAM_DEF(Vector2, target, Vector2());
	SET_GET_PARAM_DEF(bool, has_move_target, false);
	SET_GET_PARAM_DEF(bool, has_attack_target, false);

	// Info Production Queue
	SET_GET_PARAM(TypedArray<Ref<InfoProductionQueueResource>>, production_queue);

	// Basic info
	SET_GET_PARAM_DEF(String, type, "");
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
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoProxyResource, VECTOR2, position);
		ADD_SIMPLE_PROP(InfoProxyResource, VECTOR2, target);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, has_move_target);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, has_attack_target);

		ADD_ARRAY_OBJECT_PROP(InfoProxyResource, InfoProductionQueueResource, production_queue);

		ADD_SIMPLE_PROP(InfoProxyResource, STRING, type);
		ADD_SIMPLE_PROP(InfoProxyResource, INT, team);
		ADD_SIMPLE_PROP(InfoProxyResource, INT, player);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, hp);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, hp_max);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, armor);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, damage);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, reload_time);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, special);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, affinity);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, proximity_sensor_activated);
		ADD_SIMPLE_PROP(InfoProxyResource, INT, pickable_id);
		ADD_SIMPLE_PROP(InfoProxyResource, INT, rune_loads);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, alive);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, building);
	}
};

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