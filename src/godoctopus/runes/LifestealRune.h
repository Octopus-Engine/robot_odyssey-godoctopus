#pragma once

#include "flecs.h"
#include "godoctopus/runes/ModRuneData.h"
#include "godoctopus/display/particule/ParticleLibrary.h"

#include "octopus_types.h"

struct LifestealRune{
	octopus::Fixed coef[godoctopus::StatsType::StatsTypeCount];
	octopus::Fixed base = 0;
};

void declare_lifesteal_rune(flecs::world &ecs, godot::ParticleLibrary *particle_library);
LifestealRune make_lifesteal_rune(ModRuneData const&data);
