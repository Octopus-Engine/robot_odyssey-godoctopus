#pragma once

#include "flecs.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"

struct DeathParticle {
	int64_t r = 255;
	int64_t g = 255;
	int64_t b = 255;
	int64_t count = 8;
	float scale = 1.5;
	int32_t effect_id = -1;
};

void declare_death_particle_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particles);
