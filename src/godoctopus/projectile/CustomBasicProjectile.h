#pragma once

#include "flecs.h"
#include "octopus/components/basic/projectile/Projectile.hh"
#include "vat/VatLibrary.h"

#include "godoctopus/display/particule/ParticuleSmartMMesh.h"

struct CustomBasicProjectile {
	int64_t r = 255;
	int64_t g = 255;
	int64_t b = 255;
	octopus::Fixed origin_y = 1.5;
	octopus::Fixed scale = 0.2;
};

struct ProjectileTrajectory {
	octopus::Fixed target_y;
};

struct AttackParticle {
	int32_t effect_id = -1;
	float x = 0.;
	float y = 0.;
	float z = 0.;
	int64_t r = 255;
	int64_t g = 255;
	int64_t b = 255;
	int64_t count = 4;
	float scale = 0.2;
};

void declare_attack_particule_systems(flecs::world &ecs, godot::VatLibrary *library, godot::ParticuleSmartMMesh *particules);

struct WindupEffect {
	// the particule effect to be played during windup
	int32_t effect_id_load = -1;
	int32_t effect_id_incoming = -1;
	float x = 0.;
	float y = 0.;
	float z = 0.;
	int64_t r = 255;
	int64_t g = 255;
	int64_t b = 255;
	int64_t count = 4;
	float scale = 1.f;
};

void declare_windup_projectile_systems(flecs::world &ecs, godot::VatLibrary *library, godot::ParticuleSmartMMesh *particules);

void declare_basic_projectile_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particules);
