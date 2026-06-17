#pragma once

#include "core/io/resource.h"

#include "flecs.h"

#include "godot_tools.h"

namespace godot {

class PlayerResourceEntryResource : public Resource {
	GDCLASS(PlayerResourceEntryResource, Resource)

	SET_GET_PARAM_DEF(String, resource_name, "");
	SET_GET_PARAM_DEF(int64_t, amount, 0);
	SET_GET_PARAM_DEF(int64_t, cap, 0);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerResourceEntryResource, STRING, resource_name);
		ADD_SIMPLE_PROP(PlayerResourceEntryResource, INT, amount);
		ADD_SIMPLE_PROP(PlayerResourceEntryResource, INT, cap);
	}
};

class PlayerUpgradeEntryResource : public Resource {
	GDCLASS(PlayerUpgradeEntryResource, Resource)

	SET_GET_PARAM_DEF(String, upgrade_name, "");
	SET_GET_PARAM_DEF(int64_t, level, 0);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerUpgradeEntryResource, STRING, upgrade_name);
		ADD_SIMPLE_PROP(PlayerUpgradeEntryResource, INT, level);
	}
};

class PlayerLoadoutRuneEntryResource : public Resource {
	GDCLASS(PlayerLoadoutRuneEntryResource, Resource)

	SET_GET_PARAM_DEF(String, rune_internal_name, "");
	SET_GET_PARAM_DEF(String, rune_resource_path, "");
	SET_GET_PARAM_DEF(int64_t, rune_level, 1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerLoadoutRuneEntryResource, STRING, rune_internal_name);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneEntryResource, STRING, rune_resource_path);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneEntryResource, INT, rune_level);
	}
};

class PlayerLoadoutRuneSlotResource : public Resource {
	GDCLASS(PlayerLoadoutRuneSlotResource, Resource)

	SET_GET_PARAM_DEF(int32_t, slot_type, 0);
	SET_GET_PARAM_DEF(bool, activated, true);
	SET_GET_PARAM_DEF(bool, locked, false);
	SET_GET_PARAM_DEF(bool, has_rune, false);
	SET_GET_PARAM_DEF(String, rune_internal_name, "");
	SET_GET_PARAM_DEF(String, rune_resource_path, "");
	SET_GET_PARAM_DEF(int64_t, rune_level, 1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, INT, slot_type);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, BOOL, activated);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, BOOL, locked);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, BOOL, has_rune);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, STRING, rune_internal_name);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, STRING, rune_resource_path);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, INT, rune_level);
	}
};

class PlayerLoadoutUnitEntryResource : public Resource {
	GDCLASS(PlayerLoadoutUnitEntryResource, Resource)

	SET_GET_PARAM_DEF(String, prefab_name, "");
	SET_GET_PARAM(TypedArray<Ref<PlayerLoadoutRuneSlotResource>>, slots);

public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerLoadoutUnitEntryResource, STRING, prefab_name);
		ADD_ARRAY_OBJECT_PROP(PlayerLoadoutUnitEntryResource, PlayerLoadoutRuneSlotResource, slots);
	}
};

class PlayerProxyResource : public Resource {
	GDCLASS(PlayerProxyResource, Resource)

	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM(TypedArray<Ref<PlayerResourceEntryResource>>, resources);
	SET_GET_PARAM(TypedArray<Ref<PlayerUpgradeEntryResource>>, upgrades);
	SET_GET_PARAM(TypedArray<Ref<PlayerLoadoutUnitEntryResource>>, units);
	SET_GET_PARAM(TypedArray<Ref<PlayerLoadoutRuneEntryResource>>, runes);
	SET_GET_PARAM(TypedArray<String>, productions);

public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerProxyResource, INT, player);
		ADD_SIMPLE_PROP(PlayerProxyResource, INT, team);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, PlayerResourceEntryResource, resources);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, PlayerUpgradeEntryResource, upgrades);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, PlayerLoadoutUnitEntryResource, units);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, PlayerLoadoutRuneEntryResource, runes);
		ADD_ARRAY_PROP(PlayerProxyResource, productions);

		ClassDB::bind_method(D_METHOD("get_resource_amount", "resource_name"), &PlayerProxyResource::get_resource_amount);
		ClassDB::bind_method(D_METHOD("get_resource_cap", "resource_name"), &PlayerProxyResource::get_resource_cap);
		ClassDB::bind_method(D_METHOD("check_resource", "resource_name", "amount"), &PlayerProxyResource::check_resource);
		ClassDB::bind_method(D_METHOD("check_resources", "costs"), &PlayerProxyResource::check_resources);
		ClassDB::bind_method(D_METHOD("get_upgrade_level", "upgrade_name"), &PlayerProxyResource::get_upgrade_level);
		ClassDB::bind_method(D_METHOD("check_upgrade", "upgrade_name", "level"), &PlayerProxyResource::check_upgrade);
		ClassDB::bind_method(D_METHOD("check_upgrades", "requirements"), &PlayerProxyResource::check_upgrades);
	}

	int64_t get_resource_amount(String const &resource_name) const {
		TypedArray<Ref<PlayerResourceEntryResource>> const &resource_entries = get_ref_resources();
		for (int i = 0; i < resource_entries.size(); ++i) {
			Ref<PlayerResourceEntryResource> const entry = resource_entries[i];
			if (entry.is_valid() && entry->get_resource_name() == resource_name) {
				return entry->get_amount();
			}
		}
		return 0;
	}

	int64_t get_resource_cap(String const &resource_name) const {
		TypedArray<Ref<PlayerResourceEntryResource>> const &resource_entries = get_ref_resources();
		for (int i = 0; i < resource_entries.size(); ++i) {
			Ref<PlayerResourceEntryResource> const entry = resource_entries[i];
			if (entry.is_valid() && entry->get_resource_name() == resource_name) {
				return entry->get_cap();
			}
		}
		return 0;
	}

	bool check_resource(String const &resource_name, int64_t amount) const {
		return get_resource_amount(resource_name) >= amount;
	}

	bool check_resources(Dictionary const &costs) const {
		Array keys = costs.keys();
		for (int i = 0; i < keys.size(); ++i) {
			String resource_name = keys[i];
			int64_t cost_amount = (int64_t)costs[resource_name];
			if (!check_resource(resource_name, cost_amount)) {
				return false;
			}
		}
		return true;
	}

	int64_t get_upgrade_level(String const &upgrade_name) const {
		TypedArray<Ref<PlayerUpgradeEntryResource>> const &upgrade_entries = get_ref_upgrades();
		for (int i = 0; i < upgrade_entries.size(); ++i) {
			Ref<PlayerUpgradeEntryResource> const entry = upgrade_entries[i];
			if (entry.is_valid() && entry->get_upgrade_name() == upgrade_name) {
				return entry->get_level();
			}
		}
		return 0;
	}

	bool check_upgrade(String const &upgrade_name, int64_t level) const {
		return get_upgrade_level(upgrade_name) >= level;
	}

	bool check_upgrades(Dictionary const &requirements) const {
		Array keys = requirements.keys();
		for (int i = 0; i < keys.size(); ++i) {
			String upgrade_name = keys[i];
			int64_t required_level = (int64_t)requirements[upgrade_name];
			if (!check_upgrade(upgrade_name, required_level)) {
				return false;
			}
		}
		return true;
	}
};

struct PlayerProxyData {
	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM(TypedArray<Ref<PlayerResourceEntryResource>>, resources);
	SET_GET_PARAM(TypedArray<Ref<PlayerUpgradeEntryResource>>, upgrades);
	SET_GET_PARAM(TypedArray<Ref<PlayerLoadoutUnitEntryResource>>, units);
	SET_GET_PARAM(TypedArray<Ref<PlayerLoadoutRuneEntryResource>>, runes);
	SET_GET_PARAM(TypedArray<String>, productions);

public:
	Ref<PlayerProxyResource> duplicate() const {
		Ref<PlayerProxyResource> copy = Ref<PlayerProxyResource>(memnew(PlayerProxyResource));
		copy->set_player(get_player());
		copy->set_team(get_team());

		TypedArray<Ref<PlayerResourceEntryResource>> const &source_resources = get_ref_resources();
		TypedArray<Ref<PlayerResourceEntryResource>> resources_copy;
		resources_copy.resize(source_resources.size());
		for (int i = 0; i < source_resources.size(); ++i) {
			Ref<PlayerResourceEntryResource> const source = source_resources[i];
			Ref<PlayerResourceEntryResource> target = Ref<PlayerResourceEntryResource>(memnew(PlayerResourceEntryResource));
			if (source.is_valid()) {
				target->set_resource_name(source->get_resource_name());
				target->set_amount(source->get_amount());
				target->set_cap(source->get_cap());
			}
			resources_copy[i] = target;
		}
		copy->set_resources(resources_copy);

		TypedArray<Ref<PlayerUpgradeEntryResource>> const &source_upgrades = get_ref_upgrades();
		TypedArray<Ref<PlayerUpgradeEntryResource>> upgrades_copy;
		upgrades_copy.resize(source_upgrades.size());
		for (int i = 0; i < source_upgrades.size(); ++i) {
			Ref<PlayerUpgradeEntryResource> const source = source_upgrades[i];
			Ref<PlayerUpgradeEntryResource> target = Ref<PlayerUpgradeEntryResource>(memnew(PlayerUpgradeEntryResource));
			if (source.is_valid()) {
				target->set_upgrade_name(source->get_upgrade_name());
				target->set_level(source->get_level());
			}
			upgrades_copy[i] = target;
		}
		copy->set_upgrades(upgrades_copy);

		TypedArray<Ref<PlayerLoadoutUnitEntryResource>> const &source_units = get_ref_units();
		TypedArray<Ref<PlayerLoadoutUnitEntryResource>> units_copy;
		units_copy.resize(source_units.size());
		for (int i = 0; i < source_units.size(); ++i) {
			Ref<PlayerLoadoutUnitEntryResource> const source_unit = source_units[i];
			Ref<PlayerLoadoutUnitEntryResource> target_unit = Ref<PlayerLoadoutUnitEntryResource>(memnew(PlayerLoadoutUnitEntryResource));
			if (source_unit.is_valid()) {
				target_unit->set_prefab_name(source_unit->get_prefab_name());

				TypedArray<Ref<PlayerLoadoutRuneSlotResource>> const &source_slots = source_unit->get_ref_slots();
				TypedArray<Ref<PlayerLoadoutRuneSlotResource>> slots_copy;
				slots_copy.resize(source_slots.size());
				for (int slot_idx = 0; slot_idx < source_slots.size(); ++slot_idx) {
					Ref<PlayerLoadoutRuneSlotResource> const source_slot = source_slots[slot_idx];
					Ref<PlayerLoadoutRuneSlotResource> target_slot = Ref<PlayerLoadoutRuneSlotResource>(memnew(PlayerLoadoutRuneSlotResource));
					if (source_slot.is_valid()) {
						target_slot->set_slot_type(source_slot->get_slot_type());
						target_slot->set_activated(source_slot->get_activated());
						target_slot->set_locked(source_slot->get_locked());
						target_slot->set_has_rune(source_slot->get_has_rune());
						target_slot->set_rune_internal_name(source_slot->get_rune_internal_name());
						target_slot->set_rune_resource_path(source_slot->get_rune_resource_path());
						target_slot->set_rune_level(source_slot->get_rune_level());
					}
					slots_copy[slot_idx] = target_slot;
				}
				target_unit->set_slots(slots_copy);
			}
			units_copy[i] = target_unit;
		}
		copy->set_units(units_copy);

		TypedArray<Ref<PlayerLoadoutRuneEntryResource>> const &source_runes = get_ref_runes();
		TypedArray<Ref<PlayerLoadoutRuneEntryResource>> runes_copy;
		runes_copy.resize(source_runes.size());
		for (int i = 0; i < source_runes.size(); ++i) {
			Ref<PlayerLoadoutRuneEntryResource> const source_rune = source_runes[i];
			Ref<PlayerLoadoutRuneEntryResource> target_rune = Ref<PlayerLoadoutRuneEntryResource>(memnew(PlayerLoadoutRuneEntryResource));
			if (source_rune.is_valid()) {
				target_rune->set_rune_internal_name(source_rune->get_rune_internal_name());
				target_rune->set_rune_resource_path(source_rune->get_rune_resource_path());
				target_rune->set_rune_level(source_rune->get_rune_level());
			}
			runes_copy[i] = target_rune;
		}
		copy->set_runes(runes_copy);

		copy->set_productions(get_productions());

		return copy;
	}

	flecs::entity entity;
};

}
