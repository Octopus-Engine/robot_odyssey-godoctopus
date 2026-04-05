#pragma once

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"

#include <mutex>
#include "smart_list/smart_list.h"

namespace godot {

class StatsInfo : public Resource {
	GDCLASS(StatsInfo, Resource)

	SET_GET_PARAM_DEF(double, hp, 0);
	SET_GET_PARAM_DEF(double, hp_max, 0);
	SET_GET_PARAM_DEF(double, armor, 0);
	SET_GET_PARAM_DEF(double, damage, 0);
	SET_GET_PARAM_DEF(double, reload_time, 0);
	SET_GET_PARAM_DEF(double, special, 0);
	SET_GET_PARAM_DEF(double, affinity, 0);
	SET_GET_PARAM_DEF(bool, ready, false);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, hp);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, hp_max);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, armor);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, damage);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, reload_time);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, special);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, affinity);
		ADD_SIMPLE_PROP(StatsInfo, BOOL, ready);
	}
};

struct CustomUnitData {
	float x = 0.f;
	float y = 0.f;
	float visibility_range = 0.f;
};

class InfoNode : public Node {
	GDCLASS(InfoNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void setup();

	void query_stats_info(Ref<EntityGroup> group, Ref<StatsInfo> out_stats) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!group.is_valid() || !out_stats.is_valid()) {
			return;
		}
		out_stats->set_ready(false);
		_stats_info = out_stats;
		_query_entities = group->get_entities();
	}

	/// Returns a flat PackedFloat32Array of [x, y, visibility_range] triples:
	/// flecs-tracked Explorator entities first, then custom units appended.
	PackedFloat32Array get_unit_vision_data() {
		PackedFloat32Array result;
		{
			std::lock_guard<std::mutex> lock(_mutex);
			result = _unit_vision_data;
		}
		_custom_units.for_each_const([&result](CustomUnitData const &unit_data) {
			result.push_back(unit_data.x);
			result.push_back(unit_data.y);
			result.push_back(unit_data.visibility_range);
		});
		return result;
	}

	/// Adds a custom unit and returns its assigned ID.
	int add_custom_unit(float x, float y, float visibility_range);

	/// Updates an existing custom unit. No-op if the ID does not exist.
	void update_custom_unit(int id, float x, float y, float visibility_range);

	/// Removes a custom unit. No-op if the ID does not exist.
	void remove_custom_unit(int id);

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;
	Ref<StatsInfo> _stats_info;
	std::vector<flecs::entity> _query_entities;
	PackedFloat32Array _unit_vision_data;

	// Custom units are accessed from the main thread only — no locking required.
	smart_list<CustomUnitData> _custom_units;
};

}
