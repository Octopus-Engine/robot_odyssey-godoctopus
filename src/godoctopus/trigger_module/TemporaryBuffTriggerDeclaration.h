#pragma once

#include "flecs.h"

#include "octopus_types.h"

#include "godoctopus/trigger_module/buffs/TemporaryStatsBuff.h"
#include "godoctopus/components/stats/StatsSet.h"

struct TemporaryBuffInfo {
	int32_t base = 1;
	int32_t upgrade = 1;
	int64_t duration_ticks = 50;
};

// Temporary buff runes - apply buffs when reaching 3 rune load
struct ApplyHealthBuffOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; };
struct ApplyHealthBuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; int32_t range = 0; };
struct ApplyArmorBuffOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyArmorBuffOnRuneLoad, godoctopus::StatsType::Shield>; };
struct ApplyArmorBuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyArmorBuffAreaOnRuneLoad, godoctopus::StatsType::Shield>; int32_t range = 0; };
struct ApplyArmorDebuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyArmorDebuffAreaOnRuneLoad, godoctopus::StatsType::Shield>; int32_t range = 0; };
struct ApplyAttackSpeedBuffOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyAttackSpeedBuffOnRuneLoad, godoctopus::StatsType::Speed>; };
struct ApplyAttackSpeedBuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyAttackSpeedBuffAreaOnRuneLoad, godoctopus::StatsType::Speed>; int32_t range = 0; };
struct ApplyAttackSpeedDebuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyAttackSpeedDebuffAreaOnRuneLoad, godoctopus::StatsType::Speed>; int32_t range = 0; };
struct ApplyDamageBuffOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyDamageBuffOnRuneLoad, godoctopus::StatsType::Damage>; };
struct ApplyDamageBuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyDamageBuffAreaOnRuneLoad, godoctopus::StatsType::Damage>; int32_t range = 0; };
struct ApplyDamageDebuffAreaOnRuneLoad : TemporaryBuffInfo { int32_t level = 0; using BuffType = TemporaryStatsBuff<ApplyDamageDebuffAreaOnRuneLoad, godoctopus::StatsType::Damage>; int32_t range = 0; };

void declare_temporary_buff_triggers(flecs::world &ecs, custom_step_manager& manager, octopus::PositionContext const &ctx);
