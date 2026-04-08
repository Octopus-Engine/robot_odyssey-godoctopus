#pragma once

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/components/resource_producer/ResourceProducer.h"

#include <mutex>
#include <vector>

namespace godot {

/// @brief Godot Node that bridges ResourceProducer ECS events to Godot signals.
/// Call setup() on GameNode's init_done signal (automatic via init_nodes()).
/// Declares the PostUpdatePhase resource production system (replacing the free function).
class ResourceNodeEventBus : public Node {
	GDCLASS(ResourceNodeEventBus, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

	static void _bind_methods();

	void setup();
	void init_nodes();

	/// @brief Call from GDScript to fire the resource_consumed signal.
	void notify_resource_consumed(Vector3 const &position, String const &resource_name, double amount, int player);

protected:
	void _notification(int p_notification);

private:
	void _process(double delta);

	std::mutex _mutex;
	std::vector<ResourceProducedEvent> _pending_events;
};

} // namespace godot
