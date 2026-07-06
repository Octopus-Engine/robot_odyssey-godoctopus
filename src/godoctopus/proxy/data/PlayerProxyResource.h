#pragma once

#include "core/io/resource.h"

#include "godoctopus/proxy/data/player/RuneInfoResource.h"
#include "godoctopus/proxy/data/player/UnitLoadoutResource.h"
#include "godoctopus/proxy/data/PlayerResourceEntryResource.h"
#include "godoctopus/proxy/data/PlayerUpgradeEntryResource.h"
#include "godot_tools.h"

namespace godot {

class PlayerProxyResource : public Resource {
	GDCLASS(PlayerProxyResource, Resource)

	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM(TypedArray<Ref<PlayerResourceEntryResource>>, resources);
	SET_GET_PARAM(TypedArray<Ref<PlayerUpgradeEntryResource>>, upgrades);
	SET_GET_PARAM(TypedArray<Ref<UnitLoadoutResource>>, units);
	SET_GET_PARAM(TypedArray<Ref<RuneInfoResource>>, runes);
	SET_GET_PARAM(TypedArray<String>, productions);

public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerProxyResource, INT, player);
		ADD_SIMPLE_PROP(PlayerProxyResource, INT, team);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, PlayerResourceEntryResource, resources);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, PlayerUpgradeEntryResource, upgrades);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, UnitLoadoutResource, units);
		ADD_ARRAY_OBJECT_PROP(PlayerProxyResource, RuneInfoResource, runes);
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

}
