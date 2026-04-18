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

class InfoProxyResource : public Resource {
	GDCLASS(InfoProxyResource, Resource)

	SET_GET_PARAM_DEF(Vector2, position, Vector2());
	SET_GET_PARAM_DEF(Vector2, target, Vector2());
	SET_GET_PARAM_DEF(bool, has_move_target, false);
	SET_GET_PARAM_DEF(bool, has_attack_target, false);

	SET_GET_PARAM_DEF(String, type, "");
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM_DEF(double, hp, 0);
	SET_GET_PARAM_DEF(double, hp_max, 0);
	SET_GET_PARAM_DEF(double, armor, 0);
	SET_GET_PARAM_DEF(double, damage, 0);
	SET_GET_PARAM_DEF(double, reload_time, 0);
	SET_GET_PARAM_DEF(double, special, 0);
	SET_GET_PARAM_DEF(double, affinity, 0);
	SET_GET_PARAM_DEF(bool, proximity_sensor_activated, false);
	SET_GET_PARAM_DEF(bool, alive, true);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoProxyResource, VECTOR2, position);
		ADD_SIMPLE_PROP(InfoProxyResource, VECTOR2, target);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, has_move_target);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, has_attack_target);

		ADD_SIMPLE_PROP(InfoProxyResource, STRING, type);
		ADD_SIMPLE_PROP(InfoProxyResource, INT, team);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, hp);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, hp_max);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, armor);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, damage);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, reload_time);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, special);
		ADD_SIMPLE_PROP(InfoProxyResource, FLOAT, affinity);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, proximity_sensor_activated);
		ADD_SIMPLE_PROP(InfoProxyResource, BOOL, alive);
	}
};

struct InfoProxyData {
	SET_GET_PARAM(octopus::Vector, position);
	SET_GET_PARAM(octopus::Vector, target);
	SET_GET_PARAM_DEF(bool, has_move_target, false);
	SET_GET_PARAM_DEF(bool, has_attack_target, false);

	SET_GET_PARAM(std::string, type);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM_DEF(double, hp, 0);
	SET_GET_PARAM_DEF(double, hp_max, 0);
	SET_GET_PARAM_DEF(double, armor, 0);
	SET_GET_PARAM_DEF(double, damage, 0);
	SET_GET_PARAM_DEF(double, reload_time, 0);
	SET_GET_PARAM_DEF(double, special, 0);
	SET_GET_PARAM_DEF(double, affinity, 0);
	SET_GET_PARAM_DEF(bool, proximity_sensor_activated, false);
	SET_GET_PARAM_DEF(bool, alive, true);

public:
	Ref<InfoProxyResource> duplicate() const {
		Ref<InfoProxyResource> copy = Ref<InfoProxyResource>(memnew(InfoProxyResource));
		copy->set_position({(float)get_position().x.to_double(), (float)get_position().y.to_double()});
		copy->set_target({(float)get_target().x.to_double(), (float)get_target().y.to_double()});
		copy->set_has_move_target(get_has_move_target());
		copy->set_has_attack_target(get_has_attack_target());

		copy->set_type(get_type().c_str());
		copy->set_team(get_team());
		copy->set_hp(get_hp());
		copy->set_hp_max(get_hp_max());
		copy->set_armor(get_armor());
		copy->set_damage(get_damage());
		copy->set_reload_time(get_reload_time());
		copy->set_special(get_special());
		copy->set_affinity(get_affinity());
		copy->set_proximity_sensor_activated(get_proximity_sensor_activated());
		copy->set_alive(get_alive());
		return copy;
	}
};

}