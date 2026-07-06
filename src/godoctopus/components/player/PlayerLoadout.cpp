#include "PlayerLoadout.h"

#include "octopus/serialization/utils/UtilsSupport.hh"
#include "octopus/serialization/containers/VectorSupport.hh"

void declare_player_loadout_component(flecs::world &ecs) {
    ecs.component<octopus::fast_map<std::string, int64_t> >()
        .opaque(octopus::fast_map_support<std::string, int64_t>);

    ecs.component<RuneInfo>()
        .member("type", &RuneInfo::type)
        .member("data", &RuneInfo::data);

    ecs.component<std::vector<RuneInfo>>()
        .opaque(std_vector_support<RuneInfo>);

    ecs.component<PlayerRuneLoadout>()
        .member("runes", &PlayerRuneLoadout::runes);

    ecs.component<UnitRuneSlot>()
        .member("rune", &UnitRuneSlot::rune)
        .member("slot_type", &UnitRuneSlot::slot_type)
        .member("activated", &UnitRuneSlot::activated)
        .member("has_rune", &UnitRuneSlot::has_rune);

    ecs.component<std::vector<UnitRuneSlot>>()
        .opaque(std_vector_support<UnitRuneSlot>);

    ecs.component<UnitLoadout>()
        .member("prefab_name", &UnitLoadout::prefab_name)
        .member("slots", &UnitLoadout::slots);

    ecs.component<PlayerUnitLoadout>()
        .member("units", &PlayerUnitLoadout::units);
}
