#pragma once

#include "core/io/resource.h"

#include "godoctopus/proxy/data/InfoProductionQueueResource.h"
#include "godot_tools.h"

namespace godot {

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

}
