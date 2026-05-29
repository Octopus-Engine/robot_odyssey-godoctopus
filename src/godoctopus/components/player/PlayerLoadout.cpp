#include "PlayerLoadout.h"

#include "octopus/serialization/utils/UtilsSupport.hh"
#include "octopus/serialization/containers/VectorSupport.hh"

void declare_player_loadout_component(flecs::world &ecs) {
    ecs.component<PlayerRuneEntry>()
        .member("rune_internal_name", &PlayerRuneEntry::rune_internal_name)
        .member("rune_resource_path", &PlayerRuneEntry::rune_resource_path)
        .member("level", &PlayerRuneEntry::level);

    ecs.component<std::vector<PlayerRuneEntry>>()
        .opaque(std_vector_support<PlayerRuneEntry>);

    ecs.component<PlayerRuneInventory>()
        .member("runes", &PlayerRuneInventory::runes);

    ecs.component<PlayerRuneSlotData>()
        .member("slot_type", &PlayerRuneSlotData::slot_type)
        .member("locked", &PlayerRuneSlotData::locked)
        .member("has_rune", &PlayerRuneSlotData::has_rune)
        .member("rune_internal_name", &PlayerRuneSlotData::rune_internal_name)
        .member("rune_resource_path", &PlayerRuneSlotData::rune_resource_path)
        .member("rune_level", &PlayerRuneSlotData::rune_level);

    ecs.component<std::vector<PlayerRuneSlotData>>()
        .opaque(std_vector_support<PlayerRuneSlotData>);

    ecs.component<PlayerUnitLoadoutEntry>()
        .member("prefab_name", &PlayerUnitLoadoutEntry::prefab_name)
        .member("prefab_resource_path", &PlayerUnitLoadoutEntry::prefab_resource_path)
        .member("slots", &PlayerUnitLoadoutEntry::slots);

    ecs.component<std::vector<PlayerUnitLoadoutEntry>>()
        .opaque(std_vector_support<PlayerUnitLoadoutEntry>);

    ecs.component<PlayerUnitLoadout>()
        .member("units", &PlayerUnitLoadout::units);
}
