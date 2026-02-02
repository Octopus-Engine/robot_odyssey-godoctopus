#include "ProximityChecker.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/utils/aabb/aabb_tree.hh"

namespace godot {

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void ProximityChecker::_bind_methods() {
	BIND_NODE_PATH(ProximityChecker, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &ProximityChecker::setup);
	ClassDB::bind_method(D_METHOD("register_proximity_checker", "position", "range", "callable"), &ProximityChecker::register_proximity_checker);
}

void ProximityChecker::register_proximity_checker(Vector3 const &position, int range, Callable const &callable) {
	checkers.new_instance({{position.x*WORLD_SCALE, position.z*WORLD_SCALE}, range*WORLD_SCALE, callable});
}

void ProximityChecker::setup() {
	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;
	auto& pos_context = _game_node->get_world().position_context;
					std::cout<<"setup"<<std::endl;

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, &pos_context](flecs::iter&) {
			std::lock_guard<std::mutex> lock(_mutex);
			checkers.for_each([&pos_context] (Checker &checker) {
				std::function<bool(int32_t, flecs::entity)> func_l = [&checker, &pos_context](int32_t idx_l, flecs::entity e) -> bool {
					checker.triggered = true;
					return false;
				};
				tree_circle_query(pos_context.trees[0], checker.pos, checker.range, func_l);
			});
		});
}

void ProximityChecker::_process(double delta) {
	std::lock_guard<std::mutex> lock(_mutex);
	checkers.for_each([this] (Checker &checker, size_t idx) {
		if (checker.triggered) {
			checker.callable.call();
			checkers.free_instance(idx);
		}
	});
}

void ProximityChecker::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &ProximityChecker::setup));
	}
}

void ProximityChecker::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			set_process(true);
			init_nodes();
		} break;
	}
}

}
