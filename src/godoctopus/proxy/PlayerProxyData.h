#pragma once

#include "flecs.h"

#include "godoctopus/proxy/data/PlayerLoadoutRuneEntryResource.h"
#include "godoctopus/proxy/data/PlayerLoadoutRuneSlotResource.h"
#include "godoctopus/proxy/data/PlayerLoadoutUnitEntryResource.h"
#include "godoctopus/proxy/data/PlayerProxyResource.h"
#include "godoctopus/proxy/data/PlayerResourceEntryResource.h"
#include "godoctopus/proxy/data/PlayerUpgradeEntryResource.h"
#include "godot_tools.h"

namespace godot {

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
