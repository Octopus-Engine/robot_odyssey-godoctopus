#pragma once

#include <mutex>
#include <unordered_map>
#include <optional>

#include "scene/main/node.h"

#include "godoctopus/game/GameNode.h"
#include "PlayerProxyData.h"
#include "godoctopus/components/player/PlayerLoadout.h"

namespace godot {

class PlayerProxyNode;

struct PlayerProxyNodeDataLocker {
	PlayerProxyNodeDataLocker(PlayerProxyNode *node_p, std::mutex &mutex_p);

	std::unordered_map<int, PlayerProxyData> const &proxy_map;

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

	TypedArray<Ref<PlayerProxyResource>> get_proxy_from_players() const;
	Ref<PlayerProxyResource> get_proxy_from_player(int player_id) const;

	int64_t get_upgrade_level(int player_id, const String &upgrade_name) const;
	bool check_upgrade(int player_id, const String &upgrade_name, int64_t level = 1) const;

	TypedArray<Ref<PlayerLoadoutUnitEntryResource>> get_units(int player_id) const;
	TypedArray<Ref<PlayerLoadoutRuneEntryResource>> get_runes(int player_id) const;
	void set_units(int player_id, const TypedArray<Ref<PlayerLoadoutUnitEntryResource>> &units);
	void set_runes(int player_id, const TypedArray<Ref<PlayerLoadoutRuneEntryResource>> &runes);

	void add_delta_resources(int player_id, const String &resource_name, int64_t amount);
	void add_unit(int player_id, const String &prefab_name, int nb_core_slots, int nb_special_slots);
	void set_unit(int player_id, Ref<PlayerLoadoutUnitEntryResource> unit_loadout);
	void add_rune(int player_id, const String &rune_internal_name, const String &rune_resource_path, int64_t level = 1);

	void add_periodic_resource(int player_id, const String &resource_name, int64_t amount, int64_t tickrate);

	void init_nodes();

protected:
	void _notification(int p_notification);

private:
	mutable std::mutex _mutex;
	// Player Proxy info
	std::unordered_map<int, PlayerProxyData> _proxy_map;

	// Action queued up
	struct PlayerAction {
		std::unordered_map<std::string, int64_t> delta_resources;
		std::vector<PlayerUnitLoadoutEntry> added_units;
		std::vector<PlayerUnitLoadoutEntry> set_units;
		std::vector<PlayerRuneEntry> added_runes;
		std::vector<int64_t> removed_runes;
		std::optional<PlayerUnitLoadout> units_loadout;
		std::optional<PlayerRuneInventory> runes_loadout;
	};
	std::unordered_map<int, PlayerAction> player_actions;

	// Periodic resource production
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
