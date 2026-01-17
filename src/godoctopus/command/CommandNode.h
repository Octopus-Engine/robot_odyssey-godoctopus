#pragma once

#include <map>

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/pickable/Pickable.h"

namespace godot {

class CommandNode : public Node {
	GDCLASS(CommandNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

    void move_command(Ref<EntityGroup> group, Vector3 const &world_target, bool queue);
    void attack_move_command(Ref<EntityGroup> group, Vector3 const &world_target, bool queue);
    void stop_command(Ref<EntityGroup> group);
	/// @brief Select the first entity with the ability cast_name and make it perform the cast
	void cast_command(Ref<EntityGroup> group, String const &cast_name, Ref<EntityGroup> target, Vector3 const &world_target, bool queue);
	/// @brief Make all entities in group perform the cast named cast_name
	void all_cast_command(Ref<EntityGroup> group, String const &cast_name, Ref<EntityGroup> target, Vector3 const &world_target, bool queue);

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void setup();

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	octopus::Input<custom_variant, custom_step_manager> * _input_container = nullptr;
};

}
