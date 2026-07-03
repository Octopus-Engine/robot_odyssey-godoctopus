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

class InfoProxyNode;

struct InfoProxyNodeDataLocker {
	InfoProxyNodeDataLocker(InfoProxyNode *node_p, std::mutex &mutex_p);

	std::unordered_map<flecs::entity_t, InfoProxyData> const &proxy_map;

	bool is_up_to_date(EntityGroup *group) const;
private:
	InfoProxyNode *node;
	std::lock_guard<std::mutex> lock;
};

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

	/// @brief Get production queue data for entities in group. Only returns data for entities that are alive and enabled, and will return empty data for entities that are not valid.
	/// @note This is not a real-time query, but will return the last known data for the entities in the group. The data is updated at a fixed interval (every 8 ticks).
	/// @param group The entity group to query.
	/// @return A typed array of production queue resources for the entities in the group.
	TypedArray<Ref<InfoProductionQueueResource>> get_production_queue_from_group(Ref<EntityGroup> group) const;

	/// @brief Get available action capabilities for entities in group.
	/// @note This returns capability presence (union across the group), not runtime cast/production readiness.
	TypedArray<Ref<InfoAvailableActionResource>> get_available_actions_from_group(Ref<EntityGroup> group) const;

	TypedArray<Ref<InfoProductionQueueResource>> get_production_queue_from_player(int player_id) const;

	int64_t get_last_refresh_time() const { return _last_refresh_time; }

	void init_nodes();

	/// @brief Get a locker for the proxy data. The locker will lock the mutex for the duration of its lifetime, and will provide access to the proxy data map.
	/// @note This can be used to perform multiple queries on the proxy data without having to lock and unlock the mutex multiple times.
	InfoProxyNodeDataLocker get_data_locker() {
		return InfoProxyNodeDataLocker(this, _mutex);
	}

protected:
	void _notification(int p_notification);
private:
	mutable std::mutex _mutex;

	std::unordered_map<flecs::entity_t, InfoProxyData> _proxy_map;
	std::unordered_map<uint32_t, std::vector<flecs::entity_t>> _player_to_production_entities;
	int64_t _last_refresh_time = 0;

	friend struct InfoProxyNodeDataLocker;
};

}
