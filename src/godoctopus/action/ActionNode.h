#pragma once

#include <map>

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/pickable/Pickable.h"

#include <variant>
#include <mutex>

namespace godot {

struct SpawnUnitsAction {
	String prefab;
	Vector2 position;
	int team = 0;
	int count = 0;
	bool attack_move = false;
	Vector2 attack_move_target;
	Ref<EntityGroup> group;
};

struct ModRuneAction {
	String unit_type;
	String rune_type;
	int player_idx;
	bool add = false;
};

typedef std::variant<SpawnUnitsAction, ModRuneAction> Action;

class ActionNode : public Node {
	GDCLASS(ActionNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:
	enum ActionType {
		ACTION_SPAWN_UNITS,
		ACTION_BUFF_UNITS,
		ACTION_ADD_RESOURCES,
	};

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void spaw_units(String const &prefab, Vector2 const &position, int team, int count) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, false, Vector2(), current_group});
	}
	void spaw_units_attack_move(String const &prefab, Vector2 const &position, int team, int count, Vector2 const &attack_move_target) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, true, attack_move_target, current_group});
	}

	void mod_rune(String const &unit_type, String const &rune_type, int player_idx, bool add) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(ModRuneAction{unit_type, rune_type, player_idx, add});
	}

	void start_action_group(Ref<EntityGroup> group) {
		std::lock_guard<std::mutex> lock(_mutex);
		current_group = group;
	}

	void reset_action_group() {
		std::lock_guard<std::mutex> lock(_mutex);
		current_group = Ref<EntityGroup>();
	}

	void setup();

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;
	std::vector<Action> _actions;
	// reference to the current group to be populated with all new spawned units
	Ref<EntityGroup> current_group;
};

}

VARIANT_ENUM_CAST(godot::ActionNode::ActionType);
