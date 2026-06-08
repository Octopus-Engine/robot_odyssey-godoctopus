#include "PlayerProxyNode.h"

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/components/advanced/production/PlayerProduction.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"

#include "octopus_types.h"

namespace godot {

namespace {

TypedArray<Ref<PlayerLoadoutRuneEntryResource>> to_godot_runes(const PlayerRuneInventory &inventory) {
	TypedArray<Ref<PlayerLoadoutRuneEntryResource>> runes;
	runes.resize((int)inventory.runes.size());
	for (int i = 0; i < (int)inventory.runes.size(); ++i) {
		Ref<PlayerLoadoutRuneEntryResource> entry = Ref<PlayerLoadoutRuneEntryResource>(memnew(PlayerLoadoutRuneEntryResource));
		entry->set_rune_internal_name(inventory.runes[i].rune_internal_name.c_str());
		entry->set_rune_resource_path(inventory.runes[i].rune_resource_path.c_str());
		entry->set_level(inventory.runes[i].level);
		runes[i] = entry;
	}
	return runes;
}

PlayerRuneInventory from_godot_runes(const TypedArray<Ref<PlayerLoadoutRuneEntryResource>> &runes) {
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
			entry->get_level()
		});
	}
	return inventory;
}

TypedArray<Ref<PlayerLoadoutUnitEntryResource>> to_godot_units(const PlayerUnitLoadout &loadout) {
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
			slot_entry->set_locked(slot.locked);
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

PlayerUnitLoadout from_godot_units(const TypedArray<Ref<PlayerLoadoutUnitEntryResource>> &units) {
	PlayerUnitLoadout loadout;
	loadout.units.reserve((size_t)units.size());
	for (int i = 0; i < units.size(); ++i) {
		Ref<PlayerLoadoutUnitEntryResource> unit_entry = units[i];
		if (!unit_entry.is_valid()) {
			continue;
		}

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
				slot_entry->get_locked(),
				slot_entry->get_has_rune(),
				slot_entry->get_rune_internal_name().utf8().get_data(),
				slot_entry->get_rune_resource_path().utf8().get_data(),
				slot_entry->get_rune_level()
			});
		}
		loadout.units.push_back(std::move(unit));
	}
	return loadout;
}

} // namespace

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
	ClassDB::bind_method(D_METHOD("get_units", "player_id"), &PlayerProxyNode::get_units);
	ClassDB::bind_method(D_METHOD("get_runes", "player_id"), &PlayerProxyNode::get_runes);
	ClassDB::bind_method(D_METHOD("set_units", "player_id", "units"), &PlayerProxyNode::set_units);
	ClassDB::bind_method(D_METHOD("set_runes", "player_id", "runes"), &PlayerProxyNode::set_runes);
	ClassDB::bind_method(D_METHOD("add_rune", "player_id", "rune_internal_name", "rune_resource_path", "level"), &PlayerProxyNode::add_rune, DEFVAL(""), DEFVAL(1));
	ClassDB::bind_method(D_METHOD("remove_rune", "player_id", "rune_internal_name", "level"), &PlayerProxyNode::remove_rune, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("clear_runes", "player_id"), &PlayerProxyNode::clear_runes);
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

TypedArray<Ref<PlayerLoadoutUnitEntryResource>> PlayerProxyNode::get_units(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find((uint32_t)player_id);
	if (it == _proxy_map.end()) {
		return TypedArray<Ref<PlayerLoadoutUnitEntryResource>>();
	}
	return it->second.get_ref_units();
}

TypedArray<Ref<PlayerLoadoutRuneEntryResource>> PlayerProxyNode::get_runes(int player_id) const {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _proxy_map.find((uint32_t)player_id);
	if (it == _proxy_map.end()) {
		return TypedArray<Ref<PlayerLoadoutRuneEntryResource>>();
	}
	return it->second.get_ref_runes();
}

void PlayerProxyNode::set_units(int player_id, const TypedArray<Ref<PlayerLoadoutUnitEntryResource>> &units) {
	std::lock_guard<std::mutex> lock(_mutex);
	pending_units[(uint32_t)player_id] = from_godot_units(units);
}

void PlayerProxyNode::set_runes(int player_id, const TypedArray<Ref<PlayerLoadoutRuneEntryResource>> &runes) {
	std::lock_guard<std::mutex> lock(_mutex);
	pending_runes[(uint32_t)player_id] = from_godot_runes(runes);
}

void PlayerProxyNode::add_rune(int player_id, const String &rune_internal_name, const String &rune_resource_path, int64_t level) {
	std::lock_guard<std::mutex> lock(_mutex);
	uint32_t key = (uint32_t)player_id;
	PlayerRuneInventory inventory;
	auto pending_it = pending_runes.find(key);
	if (pending_it != pending_runes.end()) {
		inventory = pending_it->second;
	} else {
		auto proxy_it = _proxy_map.find(key);
		if (proxy_it != _proxy_map.end()) {
			inventory = from_godot_runes(proxy_it->second.get_ref_runes());
		}
	}
	inventory.runes.push_back({
		rune_internal_name.utf8().get_data(),
		rune_resource_path.utf8().get_data(),
		level
	});
	pending_runes[key] = std::move(inventory);
}

bool PlayerProxyNode::remove_rune(int player_id, const String &rune_internal_name, int64_t level) {
	std::lock_guard<std::mutex> lock(_mutex);
	uint32_t key = (uint32_t)player_id;
	PlayerRuneInventory inventory;
	auto pending_it = pending_runes.find(key);
	if (pending_it != pending_runes.end()) {
		inventory = pending_it->second;
	} else {
		auto proxy_it = _proxy_map.find(key);
		if (proxy_it != _proxy_map.end()) {
			inventory = from_godot_runes(proxy_it->second.get_ref_runes());
		}
	}

	std::string rune_name = rune_internal_name.utf8().get_data();
	for (auto it = inventory.runes.begin(); it != inventory.runes.end(); ++it) {
		if (it->rune_internal_name == rune_name && (level < 0 || it->level == level)) {
			inventory.runes.erase(it);
			pending_runes[key] = std::move(inventory);
			return true;
		}
	}
	return false;
}

void PlayerProxyNode::clear_runes(int player_id) {
	std::lock_guard<std::mutex> lock(_mutex);
	pending_runes[(uint32_t)player_id] = PlayerRuneInventory();
}

void PlayerProxyNode::setup() {
	if (!_game_node) {
		return;
	}

	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	flecs::query update_query = ecs.query<octopus::PlayerInfo, octopus::ResourceStock *, octopus::PlayerProduction *,octopus::PlayerUpgrade *, PlayerUnitLoadout *, PlayerRuneInventory *>();

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

			update_query.each([this](flecs::entity e, octopus::PlayerInfo &player_info, octopus::ResourceStock *resource_stock, octopus::PlayerProduction *player_production,
				octopus::PlayerUpgrade *player_upgrade, PlayerUnitLoadout *player_units, PlayerRuneInventory *player_runes) {

				auto pending_units_it = pending_units.find(player_info.idx);
				if (player_units && pending_units_it != pending_units.end()) {
					std::cout<<"updating units for player "<<player_info.idx<<std::endl;
					if (player_production) {
						std::cout<<"also updating production for player "<<player_info.idx<<std::endl;
						// Reset old loadout units
						for (auto const &unit : player_units->units) {
							player_production->productions[unit.prefab_name] = false;
							std::cout<<"updating (false) units for player "<<unit.prefab_name<<std::endl;
						}
						// Set new loadout units to production
						for (auto const &unit : pending_units_it->second.units) {
							player_production->productions[unit.prefab_name] = true;
							std::cout<<"updating (true) units for player "<<unit.prefab_name<<std::endl;
						}
					}
					*player_units = pending_units_it->second;
					pending_units.erase(pending_units_it);
				}
				auto pending_runes_it = pending_runes.find(player_info.idx);
				if (player_runes && pending_runes_it != pending_runes.end()) {
					*player_runes = pending_runes_it->second;
					pending_runes.erase(pending_runes_it);
				}

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
