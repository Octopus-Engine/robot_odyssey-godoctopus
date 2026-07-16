#pragma once

#include <map>

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/pickable/Pickable.h"
#include "godoctopus/trigger_module/TriggerDeclaration.h"

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
	bool dummy = false;
};

struct ModRuneAction {
	String unit_type;
	String rune_type;
	int player_idx;
	ModRuneData rune_data;
	bool add = false;
};

struct SpawnPropAction {
	Vector2 position;
	int ray_x100 = 100;
};

struct HpModification {
	Ref<EntityGroup> group;
	int hp_delta = 0;
};

typedef std::variant<SpawnUnitsAction, ModRuneAction, SpawnPropAction, HpModification> Action;

class ActionNode : public Node {
	GDCLASS(ActionNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void spawn_units(String const &prefab, Vector2 const &position, int team, int count) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, false, Vector2()});
	}
	void spawn_units_attack_move(String const &prefab, Vector2 const &position, int team, int count, Vector2 const &attack_move_target) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, true, attack_move_target});
	}

	void spawn_units_in_group(String const &prefab, Vector2 const &position, int team, int count, Ref<EntityGroup> group) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, false, Vector2(), group});
		if (group.is_valid()) {
			group->set_should_populate();
			group->increase_expected_population(count);
		}
	}

	void spawn_dummy_units_in_group(String const &prefab, Vector2 const &position, int team, int count, Ref<EntityGroup> group) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, false, Vector2(), group, true});
		if (group.is_valid()) {
			group->set_should_populate();
			group->increase_expected_population(count);
		}
	}
	void spawn_units_attack_move_in_group(String const &prefab, Vector2 const &position, int team, int count, Vector2 const &attack_move_target, Ref<EntityGroup> group) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnUnitsAction{prefab, position, team, count, true, attack_move_target, group});
		if (group.is_valid()) {
			group->set_should_populate();
			group->increase_expected_population(count);
		}
	}

	void mod_hp(Ref<EntityGroup> group, int hp_delta) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(HpModification{group, hp_delta});
	}

	void mod_rune(String const &unit_type, String const &rune_type, int player_idx, Dictionary rune_data, bool add);

	void spawn_prop(Vector2 const &position, int ray_x100) {
		std::lock_guard<std::mutex> lock(_mutex);
		_actions.push_back(SpawnPropAction{position, ray_x100});
	}

	void setup();

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;
	std::vector<Action> _actions;
};

}
