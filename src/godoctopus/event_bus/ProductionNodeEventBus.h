#pragma once

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/components/resource_producer/ResourceProducer.h"

namespace godot {

/// @brief Godot Node that bridges Production to Godot signals.
/// This allows to track upgrades and techs in Godot when
/// they are effective in the simulation (when the production is done).
class ProductionNodeEventBus : public Node {
	GDCLASS(ProductionNodeEventBus, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

	static void _bind_methods();

	void setup();
	void init_nodes();

	/// @brief Call from GDScript to fire the resource_consumed signal.
	void notify_resource_consumed(Vector3 const &position, String const &resource_name, double amount, int player);
	/// @brief Call from GDScript to fire the resource_produced signal.
	void notify_resource_produced(Vector3 const &position, String const &resource_name, double amount, int player);

protected:
	void _notification(int p_notification);
};

} // namespace godot
