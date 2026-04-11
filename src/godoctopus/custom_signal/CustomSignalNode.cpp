#include "CustomSignalNode.h"

#include "godoctopus/game/GameNode.h"

namespace godot {

void CustomSignalNode::_bind_methods() {
	ADD_SIGNAL(MethodInfo("custom_signal", PropertyInfo(Variant::STRING, "payload"), PropertyInfo(Variant::ARRAY, "args", PROPERTY_HINT_ARRAY_TYPE, "float")));
}

void CustomSignalNode::setup() {
	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());

	// Register singleton entity to receive custom signal events
	auto entity = _game_node->get_world().ecs.entity(CustomSignalNode::NAME());

	entity.observe<CustomSignalEvent>([this](CustomSignalEvent &event) {
		this->emit_custom_signal(event);
	});
}

void CustomSignalNode::emit_custom_signal(CustomSignalEvent &event) {
	const String signal_name = "custom_signal";
	const String payload = event.payload.c_str();
	TypedArray<float> args;
	for (float arg : event.args) {
		args.append(arg);
	}
	call_deferred("emit_signal", signal_name, payload, args);
}

void CustomSignalNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &CustomSignalNode::setup));
	}
}

void CustomSignalNode::_notification(int p_notification) {
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
