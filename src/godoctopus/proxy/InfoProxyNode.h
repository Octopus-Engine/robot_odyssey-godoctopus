#pragma once

#include <map>

#include "scene/main/node.h"

// octopus2
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "InfoProxyData.h"

#include <variant>
#include <mutex>

namespace godot {

class InfoProxyNode : public Node {
	GDCLASS(InfoProxyNode, Node)

	SET_GET_PARAM_DEF(int, refresh_tick, 8);
	SET_GET_NODE_PATH(GameNode, game_node);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void setup();
	void _process(double delta);

	/// @brief Get proxy data for entities in group. Only returns data for entities that are alive and enabled, and will return empty data for entities that are not valid.
	/// @note This is not a real-time query, but will return the last known data for the entities in the group. The data is updated at a fixed interval (every 8 ticks).
	TypedArray<Ref<InfoProxyResource>> get_proxy_from_group(Ref<EntityGroup> group) const;

	/// @note This is not a real-time query, but will return the last known data for the entities in the group. The data is updated at a fixed interval (every 8 ticks).
	TypedArray<Ref<InfoTargetResource>> get_target_from_group(Ref<EntityGroup> group) const;

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	mutable std::mutex _mutex;

	std::unordered_map<flecs::entity_t, InfoProxyData> _proxy_map;
};

}
