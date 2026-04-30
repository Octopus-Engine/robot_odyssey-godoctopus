#pragma once

#include "flecs.h"

#include "octopus_types.h"

// Temporary buff runes - apply buffs when reaching 3 rune load
struct ApplyHealthBuffOnRuneLoad { int32_t level = 0; };
struct ApplyArmorBuffOnRuneLoad { int32_t level = 0; };
struct ApplyDamageBuffOnRuneLoad { int32_t level = 0; };
struct ApplyAttackSpeedBuffOnRuneLoad { int32_t level = 0; };
struct ApplyHealthBuffAreaOnRuneLoad { int32_t level = 0; };
struct ApplyArmorBuffAreaOnRuneLoad { int32_t level = 0; };
struct ApplyDamageBuffAreaOnRuneLoad { int32_t level = 0; };
struct ApplyAttackSpeedBuffAreaOnRuneLoad { int32_t level = 0; };
struct ApplyArmorDebuffAreaOnRuneLoad { int32_t level = 0; };
struct ApplyDamageDebuffAreaOnRuneLoad { int32_t level = 0; };
struct ApplyAttackSpeedDebuffAreaOnRuneLoad { int32_t level = 0; };

void declare_temporary_buff_triggers(flecs::world &ecs, custom_step_manager& manager, octopus::PositionContext const &ctx);
