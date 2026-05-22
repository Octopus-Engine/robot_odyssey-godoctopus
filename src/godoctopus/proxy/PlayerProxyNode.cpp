#include "PlayerProxyNode.h"

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"

#include "octopus_types.h"

namespace godot {

PlayerProxyNodeDataLocker::PlayerProxyNodeDataLocker(PlayerProxyNode *node_p, std::mutex &mutex_p) :
	proxy_map(node_p->_proxy_map), lock(mutex_p) {}

void PlayerProxyNode::_bind_methods() {
	BIND_NODE_PATH(PlayerProxyNode, GameNode, game_node);
	ADD_SIMPLE_PROP(PlayerProxyNode, INT, refresh_tick);

	ClassDB::bind_method(D_METHOD("setup"), &PlayerProxyNode::setup);
	ClassDB::bind_method(D_METHOD("get_proxy_from_players"), &PlayerProxyNode::get_proxy_from_players);
	ClassDB::bind_method(D_METHOD("get_proxy_from_player", "player_id"), &PlayerProxyNode::get_proxy_from_player);
	ClassDB::bind_method(D_METHOD("get_upgrade_level", "player_id", "upgrade_name"), &PlayerProxyNode::get_upgrade_level);
	ClassDB::bind_method(D_METHOD("check_upgrade", "player_id", "upgrade_name", "level"), &PlayerProxyNode::check_upgrade, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("check_upgrades", "player_id", "requirements"), &PlayerProxyNode::check_upgrades);
	ClassDB::bind_method(D_METHOD("add_resource", "resource_name", "amount", "player_id"), &PlayerProxyNode::add_resource);
	ClassDB::bind_method(D_METHOD("add_periodic_resource", "resource_name", "amount", "tickrate", "player_id"), &PlayerProxyNode::add_periodic_resource);
}

TypedArray<Ref<PlayerResourceProxyResource>> PlayerProxyNode::get_proxy_from_players() const {
	TypedArray<Ref<PlayerResourceProxyResource>> result;
	std::lock_guard<std::mutex> lock(_mutex);
	result.resize(_proxy_map.size());
	int idx = 0;
	for (auto const &pair : _proxy_map) {
		result[idx++] = pair.second.duplicate();
	}
	return result;
}

Ref<PlayerResourceProxyResource> PlayerProxyNode::get_proxy_from_player(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find((uint32_t)player_id);
	if (it == _proxy_map.end()) {
		return Ref<PlayerResourceProxyResource>();
	}
	return it->second.duplicate();
}

int64_t PlayerProxyNode::get_upgrade_level(int player_id, const String &upgrade_name) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find((uint32_t)player_id);
	if (it == _proxy_map.end()) {
		return 0;
	}
	TypedArray<Ref<PlayerUpgradeEntryResource>> const &upgrades = it->second.get_ref_upgrades();
	for (int i = 0; i < upgrades.size(); ++i) {
		Ref<PlayerUpgradeEntryResource> const entry = upgrades[i];
		if (entry.is_valid() && entry->get_upgrade_name() == upgrade_name) {
			return entry->get_level();
		}
	}
	return 0;
}

bool PlayerProxyNode::check_upgrade(int player_id, const String &upgrade_name, int64_t level) const {
	return get_upgrade_level(player_id, upgrade_name) >= level;
}

bool PlayerProxyNode::check_upgrades(int player_id, const Dictionary &requirements) const {
	Array keys = requirements.keys();
	for (int i = 0; i < keys.size(); ++i) {
		String upgrade_name = keys[i];
		int64_t required_level = (int64_t)requirements[upgrade_name];
		if (!check_upgrade(player_id, upgrade_name, required_level)) {
			return false;
		}
	}
	return true;
}

void PlayerProxyNode::setup() {
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

void PlayerProxyNode::add_resource(const String &resource_name, int64_t amount, int player_id) {
	std::lock_guard<std::mutex> lock(_mutex);
	std::string resource_name_str = resource_name.utf8().get_data();
	added_resources[(uint32_t)player_id][resource_name_str] += amount;
}

void PlayerProxyNode::add_periodic_resource(const String &resource_name, int64_t amount, int64_t tickrate, int player_id) {
	std::lock_guard<std::mutex> lock(_mutex);
	std::string resource_name_str = resource_name.utf8().get_data();
	periodic_resources.push_back({resource_name_str, amount, tickrate, player_id});
}

void PlayerProxyNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if (_game_node) {
		_game_node->connect("init_done", callable_mp(this, &PlayerProxyNode::setup));
	}
}

void PlayerProxyNode::_process(double delta) {
	(void)delta;
}

void PlayerProxyNode::_notification(int p_notification) {
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
