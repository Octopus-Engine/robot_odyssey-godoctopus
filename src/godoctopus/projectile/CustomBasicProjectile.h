#pragma once

#include "flecs.h"
#include "octopus/components/basic/projectile/Projectile.hh"

#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
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

void declare_basic_projectile_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particules);
