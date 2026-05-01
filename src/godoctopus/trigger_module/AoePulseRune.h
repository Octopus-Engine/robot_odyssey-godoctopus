#pragma once

#include "flecs.h"

#include "octopus_types.h"

// Periodic pulse runes - heal/damage allies/enemies every TICK_RATE based on max HP or damage
struct AoePulseHealBasedOnHitpoint { int32_t level = 0; };
struct AoePulseHealBasedOnHitpointTier2 { int32_t level = 0; };
struct AoePulseHealBasedOnDamage { int32_t level = 0; };
struct AoePulseHealBasedOnDamageTier2 { int32_t level = 0; };
struct AoePulseDamageBasedOnHitpoint { int32_t level = 0; };
struct AoePulseDamageBasedOnHitpointTier2 { int32_t level = 0; };
struct AoePulseDamageBasedOnDamage { int32_t level = 0; };
struct AoePulseDamageBasedOnDamageTier2 { int32_t level = 0; };

void declare_aoe_pulse_triggers(flecs::world &ecs, octopus::PositionContext const &ctx);
