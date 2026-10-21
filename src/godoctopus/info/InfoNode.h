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

	SET_GET_PARAM_DEF(String, type, "");
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM_DEF(double, hp, 0);
	SET_GET_PARAM_DEF(double, hp_max, 0);
	SET_GET_PARAM_DEF(double, armor, 0);
	SET_GET_PARAM_DEF(double, damage, 0);
	SET_GET_PARAM_DEF(double, reload_time, 0);
	SET_GET_PARAM_DEF(double, special, 0);
	SET_GET_PARAM_DEF(double, affinity, 0);
	SET_GET_PARAM_DEF(bool, ready, false);
	SET_GET_PARAM_DEF(bool, empty, false);
	SET_GET_PARAM_DEF(bool, proximity_sensor_activated, false);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_SIMPLE_PROP(StatsInfo, STRING, type);
		ADD_SIMPLE_PROP(StatsInfo, INT, team);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, hp);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, hp_max);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, armor);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, damage);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, reload_time);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, special);
		ADD_SIMPLE_PROP(StatsInfo, FLOAT, affinity);
		ADD_SIMPLE_PROP(StatsInfo, BOOL, ready);
		ADD_SIMPLE_PROP(StatsInfo, BOOL, empty);
		ADD_SIMPLE_PROP(StatsInfo, BOOL, proximity_sensor_activated);
	}
};

class AbilityCastableInfo : public Resource {
	GDCLASS(AbilityCastableInfo, Resource)

	SET_GET_PARAM_DEF(bool, castable, false);
	SET_GET_PARAM_DEF(bool, ready, false);
	SET_GET_PARAM_DEF(String, reason, "");
	public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(AbilityCastableInfo, BOOL, castable);
		ADD_SIMPLE_PROP(AbilityCastableInfo, BOOL, ready);
		ADD_SIMPLE_PROP(AbilityCastableInfo, STRING, reason);
	}
};

/// @brief Flattened view of the command queues of a group of entities.
/// Entity i owns the waypoints [sum(counts[0..i-1]), sum(counts[0..i])) of targets/kinds.
/// kinds: 0 = move, 1 = attack move
class CommandQueueInfo : public Resource {
	GDCLASS(CommandQueueInfo, Resource)

	SET_GET_PARAM_DEF(PackedVector3Array, starts, PackedVector3Array());
	SET_GET_PARAM_DEF(PackedInt32Array, counts, PackedInt32Array());
	SET_GET_PARAM_DEF(PackedVector3Array, targets, PackedVector3Array());
	SET_GET_PARAM_DEF(PackedInt32Array, kinds, PackedInt32Array());
	SET_GET_PARAM_DEF(bool, ready, false);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(CommandQueueInfo, PACKED_VECTOR3_ARRAY, starts);
		ADD_SIMPLE_PROP(CommandQueueInfo, PACKED_INT32_ARRAY, counts);
		ADD_SIMPLE_PROP(CommandQueueInfo, PACKED_VECTOR3_ARRAY, targets);
		ADD_SIMPLE_PROP(CommandQueueInfo, PACKED_INT32_ARRAY, kinds);
		ADD_SIMPLE_PROP(CommandQueueInfo, BOOL, ready);
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

	void query_ability_castable(Ref<EntityGroup> group, String const &ability_name, Ref<AbilityCastableInfo> out_info) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!group.is_valid() || !out_info.is_valid()) {
			return;
		}
		out_info->set_ready(false);
		_ability_castable_info = out_info;
		_ability_castable_name = ability_name.utf8().get_data();
		_ability_castable_entities = group->get_entities();
	}

	/// @brief Asynchronously fills out_info with the current and queued move/attack move commands of each entity of group
	void query_command_queue(Ref<EntityGroup> group, Ref<CommandQueueInfo> out_info) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!group.is_valid() || !out_info.is_valid()) {
			return;
		}
		out_info->set_ready(false);
		_command_queue_info = out_info;
		_command_queue_entities = group->get_entities();
	}

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;
	Ref<StatsInfo> _stats_info;
	std::vector<flecs::entity> _query_entities;
	Ref<AbilityCastableInfo> _ability_castable_info;
	std::string _ability_castable_name;
	std::vector<flecs::entity> _ability_castable_entities;
	Ref<CommandQueueInfo> _command_queue_info;
	std::vector<flecs::entity> _command_queue_entities;
};

}

