#include "CommandNode.h"

namespace godot {

void CommandNode::attack_move_command(Ref<EntityGroup> group, Vector3 const &world_target, bool queue)
{
    if(!_input_container) { return; }

    octopus::AttackCommand atk_l {flecs::entity(), {world_target.x/WORLD_SCALE,world_target.z/WORLD_SCALE}, true};

	if(queue) {
        _input_container->addBackCommand(group->get_entities(), atk_l);
    }
    else {
        _input_container->addFrontCommand(group->get_entities(), atk_l);
    }
}


// Will be called by Godot when the class is registered
// Use this to add properties to your class
void CommandNode::_bind_methods() {
	BIND_NODE_PATH(CommandNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("attack_move_command", "group", "world_target", "queue"), &CommandNode::attack_move_command);
	ClassDB::bind_method(D_METHOD("setup"), &CommandNode::setup);
}

void CommandNode::setup() {
	_input_container = _game_node->get_input_controller();
}

void CommandNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &CommandNode::setup));
	}
}

void CommandNode::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
		} break;
	}
}

}
