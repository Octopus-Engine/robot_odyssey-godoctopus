#include "ProductionNodeEventBus.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"
#include "octopus/systems/phases/Phases.hh"

#include "ProductionEvent.h"

namespace godot {

void ProductionNodeEventBus::_bind_methods() {
	BIND_NODE_PATH(ProductionNodeEventBus, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &ProductionNodeEventBus::setup);

	ADD_SIGNAL(MethodInfo("production_done",
		PropertyInfo(Variant::STRING, "production_name"),
		PropertyInfo(Variant::INT, "player")));
}

void ProductionNodeEventBus::setup() {
	if (!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;
	custom_step_manager &step_manager = _game_node->get_step_context().step_manager;

	// Create an entity observer
	ecs.entity(PRODUCTION_NODE_EVENT_BUS).observe<ProductionDone>([this](ProductionDone const & prod) {
		call_deferred("emit_signal", "production_done",
				String(prod.production_name.c_str()),
				int(prod.player));
	});
}

void ProductionNodeEventBus::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if (_game_node) {
		_game_node->connect("init_done", callable_mp(this, &ProductionNodeEventBus::setup));
	}
}

void ProductionNodeEventBus::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// NA
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
			init_nodes();
		} break;
	}
}

} // namespace godot
