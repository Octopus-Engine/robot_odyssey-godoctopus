#pragma once

#include "flecs.h"
#include "godoctopus/runes/ModRuneData.h"

#include "octopus_types.h"

struct AoeOnDeathInfo {
	octopus::Fixed coef[godoctopus::StatsType::StatsTypeCount];
	octopus::Fixed base = 0;
	octopus::Fixed range = 0;
};

struct DamageOnDeathRune : AoeOnDeathInfo {};
struct HealOnDeathRune : AoeOnDeathInfo {};

void declare_aoe_on_death_runes(flecs::world &ecs, octopus::PositionContext const &ctx);
void fill_aoe_on_death_info(AoeOnDeathInfo &info, ModRuneData const&data);

template<typename RuneType>
RuneType make_aoe_on_death_rune(ModRuneData const&data) {
	RuneType rune;
	fill_aoe_on_death_info(rune, data);
	return rune;
}
