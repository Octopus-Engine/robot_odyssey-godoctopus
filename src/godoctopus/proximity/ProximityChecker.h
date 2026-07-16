#pragma once

#include <map>

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"

#include <variant>
#include <mutex>

namespace godot {

class ProximityChecker : public Node {
	GDCLASS(ProximityChecker, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void register_proximity_checker(Vector3 const &position, int range, Callable const &callable);
	void register_proximity_checker_for_team(Vector3 const &position, int range, int team, Callable const &callable);

	void register_proximity_sentry(Vector3 const &position, int range, Callable const &callable);
	void register_proximity_sentry_for_team(Vector3 const &position, int range, int team, Callable const &callable);

	void setup();
	void _process(double delta);

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;

	struct Checker {
		octopus::Vector pos;
		octopus::Fixed range;
		Callable callable;
		int tree_idx = 0;
		bool triggered = false;
	};
	smart_list<Checker> checkers;

	struct Sentry {
		octopus::Vector pos;
		octopus::Fixed range;
		Callable callable;
		int tree_idx = 0;
		bool triggered = false;
		bool value = false;
	};
	smart_list<Sentry> sentries;
};

}
