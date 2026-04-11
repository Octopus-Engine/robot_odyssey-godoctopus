#pragma once

#include "scene/main/node.h"

#include "flecs.h"

#include "godoctopus/game/GameNode.h"

namespace godot {

class GameNode;

struct CustomSignalEvent {
	std::string payload;
	std::vector<float> args; // optional additional arguments
};

class CustomSignalNode : public Node {
	GDCLASS(CustomSignalNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:
	static void _bind_methods();

	void setup();
	void emit_custom_signal(CustomSignalEvent &event);

	void init_nodes();

	static constexpr const char *NAME() { return "CustomSignalNode"; }
protected:
	void _notification(int p_notification);
};

void declare_custom_signal_bridge(flecs::world &ecs, CustomSignalNode *node);

} // namespace godot
