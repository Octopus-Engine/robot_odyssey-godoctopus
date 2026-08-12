#pragma once

#include "flecs.h"
#include "godoctopus/runes/ModRuneData.h"

#include "octopus_types.h"

struct AoePulseInfo {
	octopus::Fixed coef[godoctopus::StatsType::StatsTypeCount];
	octopus::Fixed base = 0;
	octopus::Fixed range = 0;
};

struct AoePulseDamages : AoePulseInfo {};

void declare_aoe_pulse_runes(flecs::world &ecs, octopus::PositionContext const &ctx);

void fill_aoe_pulse_info(AoePulseInfo &info, ModRuneData const&data);

template<typename RuneType>
RuneType make_aoe_pulse_rune(ModRuneData const&data) {
	RuneType rune;
	fill_aoe_pulse_info(rune, data);
	return rune;
}
