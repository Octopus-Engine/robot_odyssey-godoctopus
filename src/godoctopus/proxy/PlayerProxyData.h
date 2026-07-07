#pragma once

#include "flecs.h"

#include "godoctopus/proxy/data/PlayerProxyResource.h"
#include "godoctopus/proxy/data/PlayerResourceEntryResource.h"
#include "godoctopus/proxy/data/PlayerUpgradeEntryResource.h"
#include "godoctopus/proxy/data/player/UnitLoadoutResource.h"
#include "godoctopus/proxy/data/player/RuneInfoResource.h"
#include "godot_tools.h"

namespace godot {

struct PlayerProxyData {
	SET_GET_PARAM_DEF(int, player, 0);
	SET_GET_PARAM_DEF(int, team, 0);
	SET_GET_PARAM(TypedArray<PlayerResourceEntryResource>, resources);
	SET_GET_PARAM(TypedArray<PlayerUpgradeEntryResource>, upgrades);
	SET_GET_PARAM(TypedArray<UnitLoadoutResource>, units);
	SET_GET_PARAM(TypedArray<RuneInfoResource>, runes);
	SET_GET_PARAM(TypedArray<String>, productions);

public:
	Ref<PlayerProxyResource> duplicate() const {
		Ref<PlayerProxyResource> copy = Ref<PlayerProxyResource>(memnew(PlayerProxyResource));
		copy->set_player(get_player());
		copy->set_team(get_team());

		TypedArray<PlayerResourceEntryResource> const &source_resources = get_ref_resources();
		TypedArray<PlayerResourceEntryResource> resources_copy;
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

		TypedArray<PlayerUpgradeEntryResource> const &source_upgrades = get_ref_upgrades();
		TypedArray<PlayerUpgradeEntryResource> upgrades_copy;
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

		TypedArray<UnitLoadoutResource> const &source_units = get_ref_units();
		TypedArray<UnitLoadoutResource> units_copy;
		units_copy.resize(source_units.size());
		for (int i = 0; i < source_units.size(); ++i) {
			Ref<UnitLoadoutResource> const source = source_units[i];
			units_copy[i] = source->duplicate();
		}
		copy->set_units(units_copy);

		TypedArray<RuneInfoResource> const &source_runes = get_ref_runes();
		TypedArray<RuneInfoResource> runes_copy;
		runes_copy.resize(source_runes.size());
		for (int i = 0; i < source_runes.size(); ++i) {
			Ref<RuneInfoResource> const source = source_runes[i];
			runes_copy[i] = source->duplicate();
		}
		copy->set_runes(runes_copy);

		copy->set_productions(get_productions());

		return copy;
	}

	flecs::entity entity;
};

}
