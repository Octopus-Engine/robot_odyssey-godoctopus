#pragma once

#include "core/io/resource.h"
#include "vat/VatAnimationTrack.h"

#include "godot_tools.h"

namespace godot {

class ParticleOcherstrated : public Resource {
	GDCLASS(ParticleOcherstrated, Resource)

public:
	ParticleOcherstrated() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_SIMPLE_PROP(ParticleOcherstrated, INT, type);
		ADD_SIMPLE_PROP(ParticleOcherstrated, COLOR, color);
	}
	SET_GET_PARAM_DEF(int, type, -1);
	SET_GET_PARAM(Color, color);
};

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
		ADD_SIMPLE_PROP(UnitPrefab, STRING, description);
		ADD_SIMPLE_PROP(UnitPrefab, STRING, lore);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, exploration);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, is_static);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, is_building);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, destroyable);
		ClassDB::add_property_group("UnitPrefab", "Rune");
		// Rune
		ADD_SIMPLE_PROP(UnitPrefab, INT, special_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, affinity_x10);
		ClassDB::add_property_group("UnitPrefab", "Damage");
		// Damage
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, attack_enabled);
		ADD_SIMPLE_PROP(UnitPrefab, INT, damage_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, windup_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, reload_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, range_x10);
		ClassDB::add_property_subgroup("UnitPrefab", "BasicProjectile");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, basic_projectile);
		ADD_SIMPLE_PROP(UnitPrefab, COLOR, projectile_color);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, projectile_scale);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, projectile_origin);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, projectile_target);
		ADD_ARRAY_OBJECT_PROP(UnitPrefab, ParticleOcherstrated, impacts);
		ADD_SIMPLE_PROP(UnitPrefab, INT, impact_effect_id);
		ADD_SIMPLE_PROP(UnitPrefab, INT, impact_count);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, impact_scale);
		ClassDB::add_property_subgroup("UnitPrefab", "AttackParticle");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, attack_particle);
		ADD_SIMPLE_PROP(UnitPrefab, INT, attack_particle_effect);
		ADD_SIMPLE_PROP(UnitPrefab, VECTOR3, attack_particle_origin);
		ADD_SIMPLE_PROP(UnitPrefab, COLOR, attack_particle_color);
		ADD_SIMPLE_PROP(UnitPrefab, INT, attack_particle_count);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, attack_particle_scale);
		ClassDB::add_property_subgroup("UnitPrefab", "WindUpEffect");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, windup_effect);
		ADD_SIMPLE_PROP(UnitPrefab, INT, windup_effect_loading);
		ADD_SIMPLE_PROP(UnitPrefab, INT, windup_effect_incoming);
		ADD_SIMPLE_PROP(UnitPrefab, VECTOR3, windup_effect_origin);
		ADD_SIMPLE_PROP(UnitPrefab, COLOR, windup_effect_color);
		ADD_SIMPLE_PROP(UnitPrefab, INT, windup_effect_count);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, windup_effect_scale);
		ClassDB::add_property_group("UnitPrefab", "Display");
		// Display
		ADD_OBJECT_PROP(UnitPrefab, VatAnimationTrack, track);
		ADD_SIMPLE_PROP(UnitPrefab, INT, track_idx);
		// Topo
		ClassDB::add_property_group("UnitPrefab", "Topology");
		ADD_SIMPLE_PROP(UnitPrefab, INT, ray_x100);
		ADD_SIMPLE_PROP(UnitPrefab, INT, speed_x10);
		// Cost
		ClassDB::add_property_group("UnitPrefab", "Production");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, producer);
		ADD_SIMPLE_PROP(UnitPrefab, INT, production_point_x);
		ADD_SIMPLE_PROP(UnitPrefab, INT, production_point_y);
		ADD_SIMPLE_PROP(UnitPrefab, INT, cost_basic_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, cost_advanced_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, cost_pop_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, production_duration);
		ADD_ARRAY_OBJECT_PROP(UnitPrefab, String, required_technologies);
		// Health bar
		ClassDB::add_property_group("UnitPrefab", "Health");
		ADD_SIMPLE_PROP(UnitPrefab, INT, hitpoint);
		ADD_SIMPLE_PROP(UnitPrefab, INT, armor);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, health_bar_offset_y);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, health_bar_width);
		ClassDB::add_property_group("UnitPrefab", "DeathParticle");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, death_particles);
		ADD_SIMPLE_PROP(UnitPrefab, COLOR, death_particles_color);
		ADD_SIMPLE_PROP(UnitPrefab, INT, death_particles_count);
		ADD_SIMPLE_PROP(UnitPrefab, FLOAT, death_particles_scale);
		ADD_SIMPLE_PROP(UnitPrefab, INT, death_particles_effect_id);
		// Proximity sensor
		ClassDB::add_property_group("UnitPrefab", "ProximitySensor");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, has_proximity_sensor);
		ADD_SIMPLE_PROP(UnitPrefab, INT, proximity_sensor_range_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, proximity_sensor_refresh_rate);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, proximity_sensor_check_allies);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, proximity_sensor_dies_on_trigger);
		// Beacon
		ClassDB::add_property_group("UnitPrefab", "Beacon");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, has_beacon_ability);
		ADD_SIMPLE_PROP(UnitPrefab, STRING, beacon_producer_prefab);
		ADD_ARRAY_OBJECT_PROP(UnitPrefab, String, beacon_producer_abilities);
		// Castable Abilities
		ClassDB::add_property_group("UnitPrefab", "Abilities");
		ADD_ARRAY_OBJECT_PROP(UnitPrefab, String, castable_abilities);
		// Resource producer properties
		ClassDB::add_property_group("UnitPrefab", "ResourceProducer");
		ADD_SIMPLE_PROP(UnitPrefab, STRING, producer_resource_name);
		ADD_SIMPLE_PROP(UnitPrefab, INT, producer_resource_amount_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, producer_resource_interval);
		// Proximity Custom Signal
		ClassDB::add_property_group("UnitPrefab", "ProximityCustomSignal");
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, has_proximity_custom_signal);
		ADD_SIMPLE_PROP(UnitPrefab, INT, proximity_custom_signal_range_x10);
		ADD_SIMPLE_PROP(UnitPrefab, INT, proximity_custom_signal_refresh_rate);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, proximity_custom_signal_check_allies);
		ADD_SIMPLE_PROP(UnitPrefab, BOOL, proximity_custom_signal_dies_on_trigger);
	}

	// General
	SET_GET_PARAM(String, prefab_name);
	SET_GET_PARAM(String, description);
	SET_GET_PARAM(String, lore);
	SET_GET_PARAM_DEF(bool, exploration, false);
	SET_GET_PARAM_DEF(bool, is_static, false);
	SET_GET_PARAM_DEF(bool, is_building, false);
	SET_GET_PARAM_DEF(bool, destroyable, true);
	// Rune
	SET_GET_PARAM_DEF(int, special_x10, 0);
	SET_GET_PARAM_DEF(int, affinity_x10, 10);
	// Damage
	SET_GET_PARAM_DEF(bool, attack_enabled, true);
	SET_GET_PARAM_DEF(int, damage_x10, 30);
	SET_GET_PARAM_DEF(int, reload_x10, 10);
	SET_GET_PARAM_DEF(int, windup_x10, 10);
	SET_GET_PARAM_DEF(int, range_x10, 10);
		// Projectile
		SET_GET_PARAM_DEF(bool, basic_projectile, false);
		SET_GET_PARAM(Color, projectile_color);
		SET_GET_PARAM_DEF(float, projectile_scale, 0.2);
		SET_GET_PARAM_DEF(float, projectile_origin, 1.);
		SET_GET_PARAM_DEF(float, projectile_target, 1.);
		SET_GET_PARAM(TypedArray<Ref<ParticleOcherstrated>>, impacts);
		SET_GET_PARAM_DEF(int, impact_effect_id, -1);
		SET_GET_PARAM_DEF(int, impact_count, 4);
		SET_GET_PARAM_DEF(float, impact_scale, 0.5);
		// Attack Particle
		SET_GET_PARAM_DEF(bool, attack_particle, false);
		SET_GET_PARAM_DEF(int, attack_particle_effect, -1);
		SET_GET_PARAM(Vector3, attack_particle_origin);
		SET_GET_PARAM(Color, attack_particle_color);
		SET_GET_PARAM_DEF(int, attack_particle_count, 4);
		SET_GET_PARAM_DEF(float, attack_particle_scale, 0.2);
		// Attack Particle
		SET_GET_PARAM_DEF(bool, windup_effect, false);
		SET_GET_PARAM_DEF(int, windup_effect_loading, -1);
		SET_GET_PARAM_DEF(int, windup_effect_incoming, -1);
		SET_GET_PARAM(Vector3, windup_effect_origin);
		SET_GET_PARAM(Color, windup_effect_color);
		SET_GET_PARAM_DEF(int, windup_effect_count, 4);
		SET_GET_PARAM_DEF(float, windup_effect_scale, 0.2);
	// Display
	SET_GET_PARAM(Ref<VatAnimationTrack>, track);
	SET_GET_PARAM_DEF(int, track_idx, 0);
	// Topo
	SET_GET_PARAM_DEF(int, ray_x100, 10);
	SET_GET_PARAM_DEF(int, speed_x10, 30);
	// Production
	SET_GET_PARAM_DEF(bool, producer, false);
	SET_GET_PARAM_DEF(int, production_point_x, 0.);
	SET_GET_PARAM_DEF(int, production_point_y, 0.);
	SET_GET_PARAM_DEF(int, cost_basic_x10, 10);
	SET_GET_PARAM_DEF(int, cost_advanced_x10, 10);
	SET_GET_PARAM_DEF(int, cost_pop_x10, 10);
	SET_GET_PARAM_DEF(int, production_duration, 10);
	SET_GET_PARAM(TypedArray<String>, required_technologies);
	// Health bar
	SET_GET_PARAM_DEF(int, hitpoint, 75);
	SET_GET_PARAM_DEF(int, armor, 0);
	SET_GET_PARAM_DEF(float, health_bar_offset_y, 2.0);
	SET_GET_PARAM_DEF(float, health_bar_width, 32.);
	// Death particle
	SET_GET_PARAM_DEF(bool, death_particles, false);
	SET_GET_PARAM_DEF(Color, death_particles_color, Color(1,1,1,1));
	SET_GET_PARAM_DEF(int, death_particles_count, 8);
	SET_GET_PARAM_DEF(float, death_particles_scale, 1.5);
	SET_GET_PARAM_DEF(int, death_particles_effect_id, -1);
	// Proximity sensor
	SET_GET_PARAM_DEF(bool, has_proximity_sensor, false);
	SET_GET_PARAM_DEF(int, proximity_sensor_range_x10, 50);
	SET_GET_PARAM_DEF(int, proximity_sensor_refresh_rate, 50);
	SET_GET_PARAM_DEF(bool, proximity_sensor_check_allies, true);
	SET_GET_PARAM_DEF(bool, proximity_sensor_dies_on_trigger, false);
	// Beacon
	SET_GET_PARAM_DEF(bool, has_beacon_ability, false);
	SET_GET_PARAM(String, beacon_producer_prefab);
	SET_GET_PARAM(TypedArray<String>, beacon_producer_abilities);
	// Castable Abilities
	SET_GET_PARAM(TypedArray<String>, castable_abilities);
	// Resource producer
	SET_GET_PARAM(String, producer_resource_name);
	SET_GET_PARAM_DEF(int, producer_resource_amount_x10, 10);
	SET_GET_PARAM_DEF(int, producer_resource_interval, 50);
	// Proximity Custom Signal
	SET_GET_PARAM_DEF(bool, has_proximity_custom_signal, false);
	SET_GET_PARAM_DEF(int, proximity_custom_signal_range_x10, 50);
	SET_GET_PARAM_DEF(int, proximity_custom_signal_refresh_rate, 50);
	SET_GET_PARAM_DEF(bool, proximity_custom_signal_check_allies, true);
	SET_GET_PARAM_DEF(bool, proximity_custom_signal_dies_on_trigger, false);
};

}
