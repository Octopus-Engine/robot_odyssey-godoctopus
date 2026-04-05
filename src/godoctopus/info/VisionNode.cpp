#include "VisionNode.h"

#include "octopus/components/basic/position/Position.hh"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "godoctopus/components/Explorator.h"

namespace godot {

void VisionNode::_bind_methods() {
	BIND_NODE_PATH(VisionNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &VisionNode::setup);
	ClassDB::bind_method(D_METHOD("get_unit_vision_data"), &VisionNode::get_unit_vision_data);
	ClassDB::bind_method(D_METHOD("add_custom_unit", "x", "y", "visibility_range"), &VisionNode::add_custom_unit);
	ClassDB::bind_method(D_METHOD("update_custom_unit", "id", "x", "y", "visibility_range"), &VisionNode::update_custom_unit);
	ClassDB::bind_method(D_METHOD("remove_custom_unit", "id"), &VisionNode::remove_custom_unit);
}

void VisionNode::setup() {
	if (!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	flecs::query<octopus::Position, Explorator> vision_query = ecs.query<octopus::Position, Explorator>();

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, vision_query](flecs::iter &) {
			std::lock_guard<std::mutex> lock(_mutex);

			_unit_vision_data.clear();
			vision_query.each([this](flecs::entity, octopus::Position const &pos, Explorator const &expl) {
				_unit_vision_data.push_back(WORLD_SCALE * real_t(octopus::to_double(pos.pos.x)) + 500.);
				_unit_vision_data.push_back(WORLD_SCALE * real_t(octopus::to_double(pos.pos.y)) + 500.);
				_unit_vision_data.push_back(expl.visibility_range);
			});
		});
}

int VisionNode::add_custom_unit(float x, float y, float visibility_range) {
	smart_list_handle<CustomUnitData> handle = _custom_units.new_instance(CustomUnitData{x, y, visibility_range});
	return (int)handle.handle();
}

void VisionNode::update_custom_unit(int id, float x, float y, float visibility_range) {
	if (id >= 0 && _custom_units.is_valid((size_t)id)) {
		_custom_units.get((size_t)id) = CustomUnitData{x, y, visibility_range};
	}
}

void VisionNode::remove_custom_unit(int id) {
	if (id >= 0 && _custom_units.is_valid((size_t)id)) {
		_custom_units.free_instance((size_t)id);
	}
}

void VisionNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if (_game_node) {
		_game_node->connect("init_done", callable_mp(this, &VisionNode::setup));
	}
}

void VisionNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			init_nodes();
		} break;
	}
}

}
