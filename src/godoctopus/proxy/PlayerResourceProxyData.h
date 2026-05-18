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

class PlayerResourceProxyResource : public Resource {
	GDCLASS(PlayerResourceProxyResource, Resource)

	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM(TypedArray<Ref<PlayerResourceEntryResource>>, resources);
	SET_GET_PARAM(TypedArray<Ref<PlayerUpgradeEntryResource>>, upgrades);

public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerResourceProxyResource, INT, player);
		ADD_SIMPLE_PROP(PlayerResourceProxyResource, INT, team);
		ADD_ARRAY_OBJECT_PROP(PlayerResourceProxyResource, PlayerResourceEntryResource, resources);
		ADD_ARRAY_OBJECT_PROP(PlayerResourceProxyResource, PlayerUpgradeEntryResource, upgrades);

		ClassDB::bind_method(D_METHOD("get_resource_amount", "resource_name"), &PlayerResourceProxyResource::get_resource_amount);
		ClassDB::bind_method(D_METHOD("get_resource_cap", "resource_name"), &PlayerResourceProxyResource::get_resource_cap);
		ClassDB::bind_method(D_METHOD("check_resource", "resource_name", "amount"), &PlayerResourceProxyResource::check_resource);
		ClassDB::bind_method(D_METHOD("check_resources", "costs"), &PlayerResourceProxyResource::check_resources);
		ClassDB::bind_method(D_METHOD("get_upgrade_level", "upgrade_name"), &PlayerResourceProxyResource::get_upgrade_level);
		ClassDB::bind_method(D_METHOD("check_upgrade", "upgrade_name", "level"), &PlayerResourceProxyResource::check_upgrade);
		ClassDB::bind_method(D_METHOD("check_upgrades", "requirements"), &PlayerResourceProxyResource::check_upgrades);
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

struct PlayerResourceProxyData {
	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM(TypedArray<Ref<PlayerResourceEntryResource>>, resources);
	SET_GET_PARAM(TypedArray<Ref<PlayerUpgradeEntryResource>>, upgrades);

public:
	Ref<PlayerResourceProxyResource> duplicate() const {
		Ref<PlayerResourceProxyResource> copy = Ref<PlayerResourceProxyResource>(memnew(PlayerResourceProxyResource));
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

		return copy;
	}

	flecs::entity entity;
};

}
