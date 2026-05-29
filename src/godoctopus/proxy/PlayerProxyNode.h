#pragma once

#include <mutex>
#include <unordered_map>

#include "scene/main/node.h"

#include "godoctopus/game/GameNode.h"
#include "PlayerResourceProxyData.h"
#include "godoctopus/components/player/PlayerLoadout.h"

namespace godot {

class PlayerProxyNode;

struct PlayerProxyNodeDataLocker {
	PlayerProxyNodeDataLocker(PlayerProxyNode *node_p, std::mutex &mutex_p);

	std::unordered_map<uint32_t, PlayerResourceProxyData> const &proxy_map;

private:
	std::lock_guard<std::mutex> lock;
};

class PlayerProxyNode : public Node {
	GDCLASS(PlayerProxyNode, Node)

	SET_GET_PARAM_DEF(int, refresh_tick, 8);
	SET_GET_NODE_PATH(GameNode, game_node);

public:
	static void _bind_methods();

	void setup();
	void _process(double delta);

	TypedArray<Ref<PlayerResourceProxyResource>> get_proxy_from_players() const;
	Ref<PlayerResourceProxyResource> get_proxy_from_player(int player_id) const;
	int64_t get_upgrade_level(int player_id, const String &upgrade_name) const;
	bool check_upgrade(int player_id, const String &upgrade_name, int64_t level = 1) const;
	bool check_upgrades(int player_id, const Dictionary &requirements) const;
	TypedArray<Ref<PlayerLoadoutUnitEntryResource>> get_units(int player_id) const;
	TypedArray<Ref<PlayerLoadoutRuneEntryResource>> get_runes(int player_id) const;
	void set_units(int player_id, const TypedArray<Ref<PlayerLoadoutUnitEntryResource>> &units);
	void set_runes(int player_id, const TypedArray<Ref<PlayerLoadoutRuneEntryResource>> &runes);
	void add_rune(int player_id, const String &rune_internal_name, const String &rune_resource_path = "", int64_t level = 1);
	bool remove_rune(int player_id, const String &rune_internal_name, int64_t level = -1);
	void clear_runes(int player_id);

	PlayerProxyNodeDataLocker get_data_locker() {
		return PlayerProxyNodeDataLocker(this, _mutex);
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
	std::unordered_map<uint32_t, PlayerUnitLoadout> pending_units;
	std::unordered_map<uint32_t, PlayerRuneInventory> pending_runes;
	struct PeriodicResource {
		std::string resource_name;
		int64_t amount;
		int64_t tickrate;
		int player_id;
	};
	std::vector<PeriodicResource> periodic_resources;

	friend struct PlayerProxyNodeDataLocker;
};

}
