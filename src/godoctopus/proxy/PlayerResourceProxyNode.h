#pragma once

#include <mutex>
#include <unordered_map>

#include "scene/main/node.h"

#include "godoctopus/game/GameNode.h"
#include "PlayerResourceProxyData.h"

namespace godot {

class PlayerResourceProxyNode;

struct PlayerResourceProxyNodeDataLocker {
	PlayerResourceProxyNodeDataLocker(PlayerResourceProxyNode *node_p, std::mutex &mutex_p);

	std::unordered_map<uint32_t, PlayerResourceProxyData> const &proxy_map;

private:
	std::lock_guard<std::mutex> lock;
};

class PlayerResourceProxyNode : public Node {
	GDCLASS(PlayerResourceProxyNode, Node)

	SET_GET_PARAM_DEF(int, refresh_tick, 8);
	SET_GET_NODE_PATH(GameNode, game_node);

public:
	static void _bind_methods();

	void setup();
	void _process(double delta);

	TypedArray<Ref<PlayerResourceProxyResource>> get_proxy_from_players() const;
	Ref<PlayerResourceProxyResource> get_proxy_from_player(int player_id) const;

	PlayerResourceProxyNodeDataLocker get_data_locker() {
		return PlayerResourceProxyNodeDataLocker(this, _mutex);
	}

	void add_resource(const String &resource_name, int64_t amount, int player_id);
	void add_periodic_resource(const String &resource_name, int64_t amount, int64_t tickrate, int player_id);

	void init_nodes();

protected:
	void _notification(int p_notification);

private:
	mutable std::mutex _mutex;
	std::unordered_map<uint32_t, PlayerResourceProxyData> _proxy_map;

	std::unordered_map<uint32_t, std::unordered_map<std::string, int64_t>> added_resources;
	struct PeriodicResource {
		std::string resource_name;
		int64_t amount;
		int64_t tickrate;
		int player_id;
	};
	std::vector<PeriodicResource> periodic_resources;

	friend struct PlayerResourceProxyNodeDataLocker;
};

}
