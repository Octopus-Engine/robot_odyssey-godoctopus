#pragma once

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"

#include <mutex>

namespace godot {

class StatsInfo : public Resource {
	GDCLASS(StatsInfo, Resource)

	SET_GET_PARAM_DEF(int, team, 0);
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
		ADD_SIMPLE_PROP(StatsInfo, INT, team);
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

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;
	Ref<StatsInfo> _stats_info;
	std::vector<flecs::entity> _query_entities;
};

}

