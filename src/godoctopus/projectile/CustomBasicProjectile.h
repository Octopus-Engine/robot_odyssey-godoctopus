#pragma once

#include "flecs.h"
#include "octopus/components/basic/projectile/Projectile.hh"

#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"

struct CustomBasicProjectile {};

struct CustomBasicProjectileInfo {
	int64_t r;
	int64_t g;
	int64_t b;
};

struct ProjectileTrajectory {
	octopus::Fixed origin_y;
	octopus::Fixed target_y;
};

void declare_basic_projectile_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particules);
