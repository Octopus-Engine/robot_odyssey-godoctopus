#pragma once

#include "flecs.h"

#include "octopus_types.h"

struct AoePulseInfo {
	int32_t base = 2;
	int32_t range = 2;
	int32_t upgrade = 1;
};

// Periodic pulse runes - heal/damage allies/enemies every TICK_RATE based on max HP or damage
struct AoePulseHealBasedOnHitpoint : AoePulseInfo { int32_t level = 0; };
struct AoePulseHealBasedOnHitpointTier2 : AoePulseInfo { int32_t level = 0; };
struct AoePulseHealBasedOnDamage : AoePulseInfo { int32_t level = 0; };
struct AoePulseHealBasedOnDamageTier2 : AoePulseInfo { int32_t level = 0; };
struct AoePulseDamageBasedOnHitpoint : AoePulseInfo { int32_t level = 0; };
struct AoePulseDamageBasedOnHitpointTier2 : AoePulseInfo { int32_t level = 0; };
struct AoePulseDamageBasedOnDamage : AoePulseInfo { int32_t level = 0; };
struct AoePulseDamageBasedOnDamageTier2 : AoePulseInfo { int32_t level = 0; };

void declare_aoe_pulse_triggers(flecs::world &ecs, octopus::PositionContext const &ctx);
