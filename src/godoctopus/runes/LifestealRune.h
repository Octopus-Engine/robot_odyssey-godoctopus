#pragma once

#include "flecs.h"
#include "godoctopus/runes/ModRuneData.h"

#include "octopus_types.h"

struct LifestealRune{
	octopus::Fixed coef[godoctopus::StatsType::StatsTypeCount];
	octopus::Fixed base = 0;
};

void declare_lifesteal_rune(flecs::world &ecs);
LifestealRune make_lifesteal_rune(ModRuneData const&data);
