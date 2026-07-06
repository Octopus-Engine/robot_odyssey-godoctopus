#include "PlayerProxyNode.h"

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/components/advanced/production/PlayerProduction.hh"
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

	ClassDB::bind_method(D_METHOD("get_units", "player_id"), &PlayerProxyNode::get_units);
	ClassDB::bind_method(D_METHOD("get_runes", "player_id"), &PlayerProxyNode::get_runes);
	ClassDB::bind_method(D_METHOD("set_units", "player_id", "units"), &PlayerProxyNode::set_units);
	ClassDB::bind_method(D_METHOD("set_runes", "player_id", "runes"), &PlayerProxyNode::set_runes);

	ClassDB::bind_method(D_METHOD("add_delta_resources", "player_id", "resource_name", "amount"), &PlayerProxyNode::add_delta_resources);
	ClassDB::bind_method(D_METHOD("add_unit", "player_id", "prefab_name"), &PlayerProxyNode::add_unit);
	ClassDB::bind_method(D_METHOD("add_unit_with_slots", "player_id", "prefab_name", "nb_core_slots", "nb_special_slots"), &PlayerProxyNode::add_unit_with_slots);
	ClassDB::bind_method(D_METHOD("set_unit", "player_id", "unit_loadout"), &PlayerProxyNode::set_unit);
	ClassDB::bind_method(D_METHOD("add_rune", "player", "rune"), &PlayerProxyNode::add_rune);

	ClassDB::bind_method(D_METHOD("add_periodic_resource", "player_id", "resource_name", "amount", "tickrate"), &PlayerProxyNode::add_periodic_resource);
}

TypedArray<Ref<PlayerProxyResource>> PlayerProxyNode::get_proxy_from_players() const {
	TypedArray<Ref<PlayerProxyResource>> result;
	std::lock_guard<std::mutex> lock(_mutex);
	result.resize(_proxy_map.size());
	int idx = 0;
	for (auto const &pair : _proxy_map) {
		result[idx++] = pair.second.duplicate();
	}
	return result;
}

Ref<PlayerProxyResource> PlayerProxyNode::get_proxy_from_player(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find(player_id);
	if (it == _proxy_map.end()) {
		return Ref<PlayerProxyResource>();
	}
	return it->second.duplicate();
}

int64_t PlayerProxyNode::get_upgrade_level(int player_id, const String &upgrade_name) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find(player_id);
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

void PlayerProxyNode::add_delta_resources(int player_id, const String &resource_name, int64_t amount) {
	std::lock_guard<std::mutex> lock(_mutex);
	player_actions[player_id].delta_resources[resource_name.utf8().get_data()] += amount;
}

////////////////////
///   Loadout    ///
////////////////////

TypedArray<Ref<UnitLoadoutResource>> PlayerProxyNode::get_units(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find(player_id);
	if (it == _proxy_map.end()) {
		return TypedArray<Ref<UnitLoadoutResource>>();
	}
	return it->second.get_ref_units();
}

TypedArray<Ref<RuneInfoResource>> PlayerProxyNode::get_runes(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find(player_id);
	if (it == _proxy_map.end()) {
		return TypedArray<Ref<RuneInfoResource>>();
	}
	return it->second.get_ref_runes();
}

void PlayerProxyNode::set_units(int player_id, const TypedArray<Ref<UnitLoadoutResource>> &units) {
	std::lock_guard<std::mutex> lock(_mutex);
	PlayerUnitLoadout unit_loadout;
	for (int i = 0; i < units.size(); ++i) {
		Ref<UnitLoadoutResource> const unit_resource = units[i];
		if (unit_resource.is_valid()) {
			unit_loadout.units.push_back(unit_resource->to_data());
		}
	}
	player_actions[player_id].units_loadout = unit_loadout;
}

void PlayerProxyNode::set_runes(int player_id, const TypedArray<Ref<RuneInfoResource>> &runes) {
	std::lock_guard<std::mutex> lock(_mutex);
	PlayerRuneLoadout rune_loadout;
	for (int i = 0; i < runes.size(); ++i) {
		Ref<RuneInfoResource> const rune_resource = runes[i];
		if (rune_resource.is_valid()) {
			rune_loadout.runes.push_back(rune_resource->to_data());
		}
	}
	player_actions[player_id].runes_loadout = rune_loadout;
}

void PlayerProxyNode::add_unit(int player_id, const String &prefab_name) {
	std::lock_guard<std::mutex> lock(_mutex);
	// Add unit to player actions
	UnitLoadout unit_entry {prefab_name.utf8().get_data()};
	const int nb_activated_core_slots = _game_node->get_prefab(prefab_name)->get_core_base_rune_slots();
	const int nb_core_slots = _game_node->get_prefab(prefab_name)->get_core_max_rune_slots();
	const int nb_activated_special_slots = _game_node->get_prefab(prefab_name)->get_special_base_rune_slots();
	const int nb_special_slots = _game_node->get_prefab(prefab_name)->get_special_max_rune_slots();
	const int total_slots = nb_core_slots + nb_special_slots;
	unit_entry.slots.resize(nb_activated_core_slots, UnitRuneSlot{0, true});
	unit_entry.slots.resize(nb_core_slots, UnitRuneSlot{0, false});
	unit_entry.slots.resize(nb_core_slots+nb_activated_special_slots, UnitRuneSlot{1, true});
	unit_entry.slots.resize(total_slots, UnitRuneSlot{1, false});
	player_actions[player_id].added_units.push_back(std::move(unit_entry));
}

void PlayerProxyNode::add_unit_with_slots(int player_id, const String &prefab_name, int nb_core_slots, int nb_special_slots) {
	std::lock_guard<std::mutex> lock(_mutex);
	// Add unit to player actions
	UnitLoadout unit_entry {prefab_name.utf8().get_data()};
	unit_entry.slots.resize(nb_core_slots, UnitRuneSlot{0, true});
	unit_entry.slots.resize(nb_core_slots+nb_special_slots, UnitRuneSlot{1, true});
	player_actions[player_id].added_units.push_back(std::move(unit_entry));
}

void PlayerProxyNode::set_unit(int player_id, Ref<UnitLoadoutResource> unit_loadout) {
	std::lock_guard<std::mutex> lock(_mutex);
	player_actions[player_id].set_units.push_back(std::move(unit_loadout->to_data()));
}

void PlayerProxyNode::add_rune(int player_id, Ref<RuneInfoResource> rune) {
	std::lock_guard<std::mutex> lock(_mutex);
	if (!rune.is_valid()) {
		return;
	}
	player_actions[player_id].added_runes.push_back(std::move(rune->to_data()));
}

////////////////////
/// Loadout END  ///
////////////////////

static Ref<UnitPrefab> find_unit_prefab(GameNode *game_node, const std::string &prefab_name) {
	for (int i = 0; i < game_node->get_unit_prefabs().size(); ++i) {
		Ref<UnitPrefab> const &prefab = game_node->get_unit_prefabs()[i];
		if (prefab.is_valid() && prefab->get_prefab_name() == prefab_name.c_str()) {
			return prefab;
		}
	}
	return Ref<UnitPrefab>();
}

static void add_requirements_to_production(Ref<UnitPrefab> const &unit_prefab, octopus::PlayerProduction *player_production) {
	if (!unit_prefab.is_valid() || !player_production) {
		return;
	}
	for (int i = 0 ; i < unit_prefab->get_required_technologies().size(); ++i) {
		String const &req_name = unit_prefab->get_required_technologies()[i];
		const std::string req_name_std = req_name.utf8().get_data();
		// tier 4 requires tier 3 to be unlocked, so we add it to production as well
		if (req_name_std == "tier4") {
			player_production->productions["tier3"] = true;
		}
		// tier 3 requires tier 2 to be unlocked, so we add it to production as well
		if (req_name_std == "tier3" || req_name_std == "tier4") {
			player_production->productions["tier2"] = true;
		}
		player_production->productions[req_name_std] = true;
	}
}

void PlayerProxyNode::setup() {
	if (!_game_node) {
		return;
	}

	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	flecs::query update_query = ecs.query<octopus::PlayerInfo, octopus::ResourceStock *, octopus::PlayerProduction *,octopus::PlayerUpgrade *, PlayerUnitLoadout *, PlayerRuneLoadout *>();

	ecs.system<>()
		.kind(ecs.entity(PostUpdatePhase))
		.run([this, ecs, update_query](flecs::iter &) {
			std::lock_guard<std::mutex> lock(_mutex);
			for (const auto &periodic_resource : periodic_resources) {
				if (periodic_resource.tickrate <= 0 || octopus::get_time_stamp(ecs) % periodic_resource.tickrate != 0) {
					continue;
				}
				player_actions[periodic_resource.player_id].delta_resources[periodic_resource.resource_name] += periodic_resource.amount;
			}
		});

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, ecs, update_query](flecs::iter &) {
			if (refresh_tick > 0 && ecs.get_info()->frame_count_total % refresh_tick != 0) {
				return;
			}

			std::lock_guard<std::mutex> lock(_mutex);
			_proxy_map.clear();

			update_query.each([this](flecs::entity e, octopus::PlayerInfo &player_info, octopus::ResourceStock *resource_stock, octopus::PlayerProduction *player_production,
				octopus::PlayerUpgrade *player_upgrade, PlayerUnitLoadout *player_units, PlayerRuneLoadout *player_runes) {

				// Get pending actions for this player
				PlayerAction const &actions = player_actions[player_info.idx];

				// Update resources stock in engine
				if (resource_stock) {
					for (auto const &[resource_name, amount] : actions.delta_resources) {
						resource_stock->resource.data()[resource_name].quantity += amount;
					}
				}

				// Update units
				if (player_units && actions.units_loadout.has_value()) {
					*player_units = std::move(actions.units_loadout.value());
					// Set new loadout units to production
					if (player_production) {
						for (auto const &unit : player_units->units) {
							player_production->productions[unit.prefab_name] = true;
							// Get requirements from unit prefab and add to production if needed
							auto unit_prefab = find_unit_prefab(_game_node, unit.prefab_name);
							if (unit_prefab.is_valid()) {
								add_requirements_to_production(unit_prefab, player_production);
							}
						}
					}
				}
				else if (player_units) {
					// Reset old loadout units production
					if (player_production) {
						for (auto const &unit : player_units->units) {
							player_production->productions[unit.prefab_name] = false;
						}
					}
					// Add new units to loadout
					for (auto const &unit : actions.added_units) {
						player_units->units.push_back(unit);
						// Get requirements from unit prefab and add to production if needed
						auto unit_prefab = find_unit_prefab(_game_node, unit.prefab_name);
						if (unit_prefab.is_valid()) {
							add_requirements_to_production(unit_prefab, player_production);
						}
					}
					// Set new loadout units to production
					if (player_production) {
						for (auto const &unit : player_units->units) {
							player_production->productions[unit.prefab_name] = true;
						}
					}

					// Set units in loadout
					for (auto const &unit : actions.set_units) {
						// Find iterator
						auto it = std::find_if(player_units->units.begin(), player_units->units.end(), [&unit](const UnitLoadout &entry) {
							return entry.prefab_name == unit.prefab_name;
						});
						// Affect it
						if (it != player_units->units.end()) {
							*it = unit;
						} else {
							// If not found, add it to loadout
							player_units->units.push_back(unit);
							// Also update production if needed
							if (player_production) {
								player_production->productions[unit.prefab_name] = true;
							}
						}
					}
				}

				// Update runes
				if (player_runes && actions.runes_loadout.has_value()) {
					*player_runes = std::move(actions.runes_loadout.value());
				} else if (player_runes) {
					// Add new runes to inventory
					for (auto const &rune : actions.added_runes) {
						player_runes->runes.push_back(rune);
					}
				}

				//////////////////////////
				//  Update proxy data   //
				//////////////////////////

				PlayerProxyData &proxy_data = _proxy_map[player_info.idx];
				proxy_data.entity = e;
				proxy_data.set_player((int)player_info.idx);
				proxy_data.set_team((int)player_info.team);

				// resources
				TypedArray<Ref<PlayerResourceEntryResource>> resources;
				if (resource_stock) {
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

				// Player upgrades
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

				// Sync loadout units and runes
				if (player_units) {
					TypedArray<Ref<UnitLoadoutResource>> units;
					for (auto const &unit : player_units->units) {
						Ref<UnitLoadoutResource> entry = Ref<UnitLoadoutResource>(memnew(UnitLoadoutResource));
						entry->from_data(unit);
						units.append(entry);
					}
					proxy_data.set_units(units);
				} else {
					proxy_data.set_units(TypedArray<Ref<UnitLoadoutResource>>());
				}
				if (player_runes) {
					TypedArray<Ref<RuneInfoResource>> runes;
					for (auto const &rune : player_runes->runes) {
						Ref<RuneInfoResource> entry = Ref<RuneInfoResource>(memnew(RuneInfoResource));
						entry->from_data(rune);
						runes.append(entry);
					}
					proxy_data.set_runes(runes);
				} else {
					proxy_data.set_runes(TypedArray<Ref<RuneInfoResource>>());
				}

				// Update production
				TypedArray<String> productions;
				if (player_production) {
					for (auto const &[production_name, production_enabled] : player_production->productions.data()) {
						if (production_enabled) {
							productions.append(String(production_name.c_str()));
						}
					}
					proxy_data.set_productions(productions);
				}

				player_actions[player_info.idx] = PlayerAction(); // Clear pending actions after applying them
			});
		});
}

void PlayerProxyNode::add_periodic_resource(int player_id, const String &resource_name, int64_t amount, int64_t tickrate) {
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
