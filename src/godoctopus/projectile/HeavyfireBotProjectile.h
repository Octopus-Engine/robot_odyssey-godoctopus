#pragma once

#include "flecs.h"
#include "octopus/components/basic/projectile/Projectile.hh"
#include "godoctopus/components/types/Types.h"
#include "octopus/world/WorldContext.hh"

#include "octopus_types.h"

struct HeavyfireBotProjectile {
	octopus::Fixed aoe_damage;
	octopus::Fixed range;
};

void declare_heavyfire_bot_projectile_systems(flecs::world &ecs, octopus::PositionContext const &ctx, custom_step_manager &manager);
