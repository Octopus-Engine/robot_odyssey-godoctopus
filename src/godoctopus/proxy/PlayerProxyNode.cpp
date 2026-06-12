#include "PlayerProxyNode.h"

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/components/advanced/production/PlayerProduction.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"

#include "octopus_types.h"

namespace godot {

static TypedArray<Ref<PlayerLoadoutRuneEntryResource>> to_godot_runes(const PlayerRuneInventory &inventory) {
	TypedArray<Ref<PlayerLoadoutRuneEntryResource>> runes;
	runes.resize((int)inventory.runes.size());
	for (int i = 0; i < (int)inventory.runes.size(); ++i) {
		Ref<PlayerLoadoutRuneEntryResource> entry = Ref<PlayerLoadoutRuneEntryResource>(memnew(PlayerLoadoutRuneEntryResource));
		entry->set_rune_internal_name(inventory.runes[i].rune_internal_name.c_str());
		entry->set_rune_resource_path(inventory.runes[i].rune_resource_path.c_str());
		entry->set_rune_level(inventory.runes[i].level);
		runes[i] = entry;
	}
	return runes;
}

static PlayerRuneInventory from_godot_runes(const TypedArray<Ref<PlayerLoadoutRuneEntryResource>> &runes) {
	PlayerRuneInventory inventory;
	inventory.runes.reserve((size_t)runes.size());
	for (int i = 0; i < runes.size(); ++i) {
		Ref<PlayerLoadoutRuneEntryResource> entry = runes[i];
		if (!entry.is_valid()) {
			continue;
		}
		inventory.runes.push_back({
			entry->get_rune_internal_name().utf8().get_data(),
			entry->get_rune_resource_path().utf8().get_data(),
			entry->get_rune_level()
		});
	}
	return inventory;
}

static TypedArray<Ref<PlayerLoadoutUnitEntryResource>> to_godot_units(const PlayerUnitLoadout &loadout) {
	TypedArray<Ref<PlayerLoadoutUnitEntryResource>> units;
	units.resize((int)loadout.units.size());
	for (int i = 0; i < (int)loadout.units.size(); ++i) {
		Ref<PlayerLoadoutUnitEntryResource> unit_entry = Ref<PlayerLoadoutUnitEntryResource>(memnew(PlayerLoadoutUnitEntryResource));
		unit_entry->set_prefab_name(loadout.units[i].prefab_name.c_str());

		TypedArray<Ref<PlayerLoadoutRuneSlotResource>> slots;
		slots.resize((int)loadout.units[i].slots.size());
		for (int slot_idx = 0; slot_idx < (int)loadout.units[i].slots.size(); ++slot_idx) {
			Ref<PlayerLoadoutRuneSlotResource> slot_entry = Ref<PlayerLoadoutRuneSlotResource>(memnew(PlayerLoadoutRuneSlotResource));
			const PlayerRuneSlotData &slot = loadout.units[i].slots[slot_idx];
			slot_entry->set_slot_type(slot.slot_type);
			slot_entry->set_locked(slot.has_rune);
			slot_entry->set_has_rune(slot.has_rune);
			slot_entry->set_rune_internal_name(slot.rune_internal_name.c_str());
			slot_entry->set_rune_resource_path(slot.rune_resource_path.c_str());
			slot_entry->set_rune_level(slot.rune_level);
			slots[slot_idx] = slot_entry;
		}
		unit_entry->set_slots(slots);
		units[i] = unit_entry;
	}
	return units;
}

static PlayerUnitLoadoutEntry from_godot_unit(const Ref<PlayerLoadoutUnitEntryResource> &unit_entry) {
	PlayerUnitLoadoutEntry unit;
	unit.prefab_name = unit_entry->get_prefab_name().utf8().get_data();
	TypedArray<Ref<PlayerLoadoutRuneSlotResource>> slots = unit_entry->get_ref_slots();
	unit.slots.reserve((size_t)slots.size());
	for (int slot_idx = 0; slot_idx < slots.size(); ++slot_idx) {
		Ref<PlayerLoadoutRuneSlotResource> slot_entry = slots[slot_idx];
		if (!slot_entry.is_valid()) {
			continue;
		}
		unit.slots.push_back({
			slot_entry->get_slot_type(),
			slot_entry->get_has_rune(),
			slot_entry->get_rune_internal_name().utf8().get_data(),
			slot_entry->get_rune_resource_path().utf8().get_data(),
			slot_entry->get_rune_level()
		});
	}
	return unit;
}

static PlayerUnitLoadout from_godot_units(const TypedArray<Ref<PlayerLoadoutUnitEntryResource>> &units) {
	PlayerUnitLoadout loadout;
	loadout.units.reserve((size_t)units.size());
	for (int i = 0; i < units.size(); ++i) {
		Ref<PlayerLoadoutUnitEntryResource> unit_entry = units[i];
		if (!unit_entry.is_valid()) {
			continue;
		}

		PlayerUnitLoadoutEntry unit = from_godot_unit(unit_entry);
		loadout.units.push_back(std::move(unit));
	}
	return loadout;
}

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
	ClassDB::bind_method(D_METHOD("add_unit", "player_id", "prefab_name", "nb_core_slots", "nb_special_slots"), &PlayerProxyNode::add_unit);
	ClassDB::bind_method(D_METHOD("set_unit", "player_id", "unit_loadout"), &PlayerProxyNode::set_unit);
	ClassDB::bind_method(D_METHOD("add_rune", "player_id", "rune_internal_name", "rune_resource_path", "level"), &PlayerProxyNode::add_rune, DEFVAL(1));

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

TypedArray<Ref<PlayerLoadoutUnitEntryResource>> PlayerProxyNode::get_units(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find(player_id);
	if (it == _proxy_map.end()) {
		return TypedArray<Ref<PlayerLoadoutUnitEntryResource>>();
	}
	return it->second.get_ref_units();
}

TypedArray<Ref<PlayerLoadoutRuneEntryResource>> PlayerProxyNode::get_runes(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find(player_id);
	if (it == _proxy_map.end()) {
		return TypedArray<Ref<PlayerLoadoutRuneEntryResource>>();
	}
	return it->second.get_ref_runes();
}

void PlayerProxyNode::set_units(int player_id, const TypedArray<Ref<PlayerLoadoutUnitEntryResource>> &units) {
	std::lock_guard<std::mutex> lock(_mutex);
	player_actions[player_id].units_loadout = from_godot_units(units);
}

void PlayerProxyNode::set_runes(int player_id, const TypedArray<Ref<PlayerLoadoutRuneEntryResource>> &runes) {
	std::lock_guard<std::mutex> lock(_mutex);
	player_actions[player_id].runes_loadout = from_godot_runes(runes);
}

void PlayerProxyNode::add_delta_resources(int player_id, const String &resource_name, int64_t amount) {
	std::lock_guard<std::mutex> lock(_mutex);
	player_actions[player_id].delta_resources[resource_name.utf8().get_data()] += amount;
}

void PlayerProxyNode::add_unit(int player_id, const String &prefab_name, int nb_core_slots, int nb_special_slots) {
	std::lock_guard<std::mutex> lock(_mutex);
	// Add unit to player actions
	PlayerUnitLoadoutEntry unit_entry {prefab_name.utf8().get_data()};
	unit_entry.slots.resize(nb_core_slots, PlayerRuneSlotData{0});
	unit_entry.slots.resize(nb_core_slots+nb_special_slots, PlayerRuneSlotData{1});
	player_actions[player_id].added_units.push_back(std::move(unit_entry));
}

void PlayerProxyNode::set_unit(int player_id, Ref<PlayerLoadoutUnitEntryResource> unit_loadout) {
	std::lock_guard<std::mutex> lock(_mutex);
	PlayerUnitLoadoutEntry unit_entry = from_godot_unit(unit_loadout);
	player_actions[player_id].set_units.push_back(std::move(unit_entry));
}

void PlayerProxyNode::add_rune(int player_id, const String &rune_internal_name, const String &rune_resource_path, int64_t level) {
	std::lock_guard<std::mutex> lock(_mutex);
	player_actions[player_id].added_runes.push_back({
		rune_internal_name.utf8().get_data(),
		rune_resource_path.utf8().get_data(),
		level
	});
}

void PlayerProxyNode::setup() {
	if (!_game_node) {
		return;
	}

	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	flecs::query update_query = ecs.query<octopus::PlayerInfo, octopus::ResourceStock *, octopus::PlayerProduction *,octopus::PlayerUpgrade *, PlayerUnitLoadout *, PlayerRuneInventory *>();

	ecs.system<>()
		.kind(ecs.entity(PostUpdatePhase))
		.run([this, ecs, update_query](flecs::iter &) {
			std::lock_guard<std::mutex> lock(_mutex);
			_proxy_map.clear();

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
				octopus::PlayerUpgrade *player_upgrade, PlayerUnitLoadout *player_units, PlayerRuneInventory *player_runes) {

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
						auto it = std::find_if(player_units->units.begin(), player_units->units.end(), [&unit](const PlayerUnitLoadoutEntry &entry) {
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

				// Loadout units and runes
				if (player_units) {
					proxy_data.set_units(to_godot_units(*player_units));
				} else {
					proxy_data.set_units(TypedArray<Ref<PlayerLoadoutUnitEntryResource>>());
				}
				if (player_runes) {
					proxy_data.set_runes(to_godot_runes(*player_runes));
				} else {
					proxy_data.set_runes(TypedArray<Ref<PlayerLoadoutRuneEntryResource>>());
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
