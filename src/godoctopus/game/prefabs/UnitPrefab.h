#pragma once

#include "core/io/resource.h"
#include "vat/VatAnimationTrack.h"

#include "godot_tools.h"

namespace godot {

class UnitPrefab : public Resource {
	GDCLASS(UnitPrefab, Resource)

public:
	UnitPrefab() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods()
	{
		ClassDB::add_property_group("UnitPrefab", "General");
		// General
		ADD_SIMPLE_PROP(UnitPrefab, STRING, prefab_name);
		ClassDB::add_property_group("UnitPrefab", "Damage");
		// Damage
		ADD_SIMPLE_PROP(UnitPrefab, INT, damage_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, windup_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, reload_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, range_x10);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, basic_projectile);
		ADD_SIMPLE_PROP(UnitPrefab, COLOR, projectile_color);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, projectile_scale);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, projectile_origin);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, projectile_target);
		ClassDB::add_property_group("UnitPrefab", "Display");
		// Display
		ADD_OBJECT_PROP(UnitPrefab, VatAnimationTrack, track);
		ADD_SIMPLE_PROP(UnitPrefab, INT, track_idx);
		// Topo
		ClassDB::add_property_group("UnitPrefab", "Topology");
		ADD_SIMPLE_PROP(UnitPrefab, INT, ray_x100);
		ADD_SIMPLE_PROP(UnitPrefab, INT, speed_x10);
		// Cost
		ClassDB::add_property_group("UnitPrefab", "Cost");
		ADD_SIMPLE_PROP(UnitPrefab, INT, cost_basic_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, cost_advanced_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, cost_pop_x10);
		// Health bar
		ClassDB::add_property_group("UnitPrefab", "Health");
		ADD_SIMPLE_PROP(UnitPrefab, INT, hitpoint);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, health_bar_offset_y);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, health_bar_width);
	}

	// General
	SET_GET_PARAM(String, prefab_name);
	// Damage
	SET_GET_PARAM_DEF(int, damage_x10, 30);
	SET_GET_PARAM_DEF(int, reload_x10, 10);
	SET_GET_PARAM_DEF(int, windup_x10, 10);
	SET_GET_PARAM_DEF(int, range_x10, 10);
	SET_GET_PARAM_DEF(bool, basic_projectile, false);
	SET_GET_PARAM(Color, projectile_color);
	SET_GET_PARAM_DEF(float, projectile_scale, 0.2);
	SET_GET_PARAM_DEF(float, projectile_origin, 1.);
	SET_GET_PARAM_DEF(float, projectile_target, 1.);
	// Display
	SET_GET_PARAM(Ref<VatAnimationTrack>, track);
	SET_GET_PARAM_DEF(int, track_idx, 0);
	// Topo
	SET_GET_PARAM_DEF(int, ray_x100, 10);
	SET_GET_PARAM_DEF(int, speed_x10, 30);
	// Cost
	SET_GET_PARAM_DEF(int, cost_basic_x10, 10);
	SET_GET_PARAM_DEF(int, cost_advanced_x10, 10);
	SET_GET_PARAM_DEF(int, cost_pop_x10, 10);
	// Health bar
	SET_GET_PARAM_DEF(int, hitpoint, 75);
	SET_GET_PARAM_DEF(float, health_bar_offset_y, 2.0);
	SET_GET_PARAM_DEF(float, health_bar_width, 32.);
};

}
