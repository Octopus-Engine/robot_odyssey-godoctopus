#include "PlayerResourceProxyNode.h"

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"

#include "octopus_types.h"

namespace godot {

PlayerResourceProxyNodeDataLocker::PlayerResourceProxyNodeDataLocker(PlayerResourceProxyNode *node_p, std::mutex &mutex_p) :
	proxy_map(node_p->_proxy_map), lock(mutex_p) {}

void PlayerResourceProxyNode::_bind_methods() {
	BIND_NODE_PATH(PlayerResourceProxyNode, GameNode, game_node);
	ADD_SIMPLE_PROP(PlayerResourceProxyNode, INT, refresh_tick);

	ClassDB::bind_method(D_METHOD("setup"), &PlayerResourceProxyNode::setup);
	ClassDB::bind_method(D_METHOD("get_proxy_from_players"), &PlayerResourceProxyNode::get_proxy_from_players);
	ClassDB::bind_method(D_METHOD("get_proxy_from_player", "player_id"), &PlayerResourceProxyNode::get_proxy_from_player);
	ClassDB::bind_method(D_METHOD("add_resource", "resource_name", "amount", "player_id"), &PlayerResourceProxyNode::add_resource);
	ClassDB::bind_method(D_METHOD("add_periodic_resource", "resource_name", "amount", "tickrate", "player_id"), &PlayerResourceProxyNode::add_periodic_resource);
}

TypedArray<Ref<PlayerResourceProxyResource>> PlayerResourceProxyNode::get_proxy_from_players() const {
	TypedArray<Ref<PlayerResourceProxyResource>> result;
	std::lock_guard<std::mutex> lock(_mutex);
	result.resize(_proxy_map.size());
	int idx = 0;
	for (auto const &pair : _proxy_map) {
		result[idx++] = pair.second.duplicate();
	}
	return result;
}

Ref<PlayerResourceProxyResource> PlayerResourceProxyNode::get_proxy_from_player(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find((uint32_t)player_id);
	if (it == _proxy_map.end()) {
		return Ref<PlayerResourceProxyResource>();
	}
	return it->second.duplicate();
}

void PlayerResourceProxyNode::setup() {
	if (!_game_node) {
		return;
	}

	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	flecs::query<octopus::PlayerInfo, octopus::ResourceStock *, octopus::PlayerUpgrade *> update_query =
		ecs.query<octopus::PlayerInfo, octopus::ResourceStock *, octopus::PlayerUpgrade *>();

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, ecs, update_query](flecs::iter &) {
			if (refresh_tick > 0 && octopus::get_time_stamp(ecs) % refresh_tick != 0) {
				return;
			}

			std::lock_guard<std::mutex> lock(_mutex);
			_proxy_map.clear();

			for (const auto &periodic_resource : periodic_resources) {
				if (periodic_resource.tickrate <= 0 || octopus::get_time_stamp(ecs) % periodic_resource.tickrate != 0) {
					continue;
				}
				added_resources[(uint32_t)periodic_resource.player_id][periodic_resource.resource_name] += periodic_resource.amount;
			}

			update_query.each([this](flecs::entity e, octopus::PlayerInfo &player_info, octopus::ResourceStock *resource_stock, octopus::PlayerUpgrade *player_upgrade) {
				PlayerResourceProxyData &proxy_data = _proxy_map[player_info.idx];
				proxy_data.entity = e;
				proxy_data.set_player((int)player_info.idx);
				proxy_data.set_team((int)player_info.team);

				TypedArray<Ref<PlayerResourceEntryResource>> resources;
				if (resource_stock) {
					auto it = added_resources.find(player_info.idx);
					if (it != added_resources.end()) {
						for (auto const &[resource_name, amount] : it->second) {
							resource_stock->resource.data()[resource_name].quantity += amount;
						}
					}

					resources.resize((int)resource_stock->resource.data().size());
					int idx = 0;
					for (auto const &[resource_name, resource_info] : resource_stock->resource.data()) {
						Ref<PlayerResourceEntryResource> entry = Ref<PlayerResourceEntryResource>(memnew(PlayerResourceEntryResource));
						entry->set_resource_name(resource_name.c_str());
						entry->set_amount(resource_info.quantity.to_int());
						entry->set_cap(resource_info.cap.to_int());
						resources[idx++] = entry;
					}
				}
				proxy_data.set_resources(resources);

				TypedArray<Ref<PlayerUpgradeEntryResource>> upgrades;
				if (player_upgrade) {
					upgrades.resize((int)player_upgrade->upgrades.data().size());
					int idx = 0;
					for (auto const &[upgrade_name, upgrade_level] : player_upgrade->upgrades.data()) {
						Ref<PlayerUpgradeEntryResource> entry = Ref<PlayerUpgradeEntryResource>(memnew(PlayerUpgradeEntryResource));
						entry->set_upgrade_name(upgrade_name.c_str());
						entry->set_level(upgrade_level);
						upgrades[idx++] = entry;
					}
				}
				proxy_data.set_upgrades(upgrades);
			});

			added_resources.clear();
		});
}

void PlayerResourceProxyNode::add_resource(const String &resource_name, int64_t amount, int player_id) {
	std::lock_guard<std::mutex> lock(_mutex);
	std::string resource_name_str = resource_name.utf8().get_data();
	added_resources[(uint32_t)player_id][resource_name_str] += amount;
}

void PlayerResourceProxyNode::add_periodic_resource(const String &resource_name, int64_t amount, int64_t tickrate, int player_id) {
	std::lock_guard<std::mutex> lock(_mutex);
	std::string resource_name_str = resource_name.utf8().get_data();
	periodic_resources.push_back({resource_name_str, amount, tickrate, player_id});
}

void PlayerResourceProxyNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if (_game_node) {
		_game_node->connect("init_done", callable_mp(this, &PlayerResourceProxyNode::setup));
	}
}

void PlayerResourceProxyNode::_process(double delta) {
	(void)delta;
}

void PlayerResourceProxyNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
			init_nodes();
		} break;
	}
}

}
