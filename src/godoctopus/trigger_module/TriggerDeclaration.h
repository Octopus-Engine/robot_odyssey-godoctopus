#pragma once

#include "flecs.h"
#include "octopus/world/position/PositionContext.hh"

#include "godoctopus/trigger_module/buffs/ArmorBuff.h"
#include "godoctopus/trigger_module/buffs/AttackSpeedBuff.h"
#include "godoctopus/trigger_module/buffs/DamageBuff.h"
#include "godoctopus/trigger_module/buffs/HitPointBuff.h"
#include "godoctopus/trigger_module/buffs/SpecialBuff.h"
#include "godoctopus/trigger_module/buffs/ScalingBuff.h"
#include "godoctopus/trigger_module/buffs/ScalingConditionalBuff.h"
#include "godoctopus/trigger_module/buffs/TemporaryStatsBuff.h"

#include "octopus_types.h"

namespace godot {
	class SmartMMeshLibrary;
}

struct AddRuneLoadOnAttack { int32_t level = 0; };
struct HealAndConsumeRuneLoadOnHit { int32_t level = 0; };
struct HealAndConsumeRuneLoadOnAttack { int32_t level = 0; };
struct BonusDamageConsumeRuneOnAttack { int32_t level = 0; };
struct BonusDamageRune { int32_t level = 0; };
struct AddRuneLoadOnHit { int32_t level = 0; };
struct BonusDamageConsumeRuneOnTargetOnAttack { int32_t level = 0; };
struct LifestealRuneCore { int32_t level = 0; };

struct ScalingRuneInfo {
	int32_t base = 0;
	int32_t upgrade = 0;
};

struct AreaScalingRuneInfo : ScalingRuneInfo {
	int32_t range = 0;
};

struct AddRuneLoadOnTargetOnAttack : ScalingRuneInfo { int32_t level = 0; };
struct LifestealRuneSpecial : ScalingRuneInfo { int32_t level = 0; };
struct AoeDamageSpecial : AreaScalingRuneInfo { int32_t level = 0; };
struct AoeDamageBasedOnHitpointOnDeath : AreaScalingRuneInfo { int32_t level = 0; };
struct AoeDamageBasedOnDamageOnDeath : AreaScalingRuneInfo { int32_t level = 0; };
struct AoeHealBasedOnHitpointOnDeath : AreaScalingRuneInfo { int32_t level = 0; };
struct AoeHealBasedOnDamageOnDeath : AreaScalingRuneInfo { int32_t level = 0; };

struct AoeDamageConsumeRuneOnHit : AreaScalingRuneInfo { int32_t level = 0; };

// Regular buffs runes
struct DamageBuffRuneRegular : FlatBuff<LeveledStatBuff<godoctopus::StatsType::Damage>, godoctopus::BaseStats> { int32_t level = 0; };
struct HitPointBuffRuneRegular : FlatBuff<LeveledStatBuff<godoctopus::StatsType::HitPoints>, godoctopus::BaseStats> { int32_t level = 0; };
struct ReloadBuffRuneRegular : FlatBuff<LeveledStatBuff<godoctopus::StatsType::Speed>, godoctopus::BaseStats> { int32_t level = 0; };
struct ArmorBuffRuneRegular : FlatBuff<LeveledStatBuff<godoctopus::StatsType::Shield>, godoctopus::BaseStats> { int32_t level = 0; };
struct SpecialBuffRuneRegular : FlatBuff<LeveledSpecialBuff, Special> { int32_t level = 0; };
struct AffinityBuffRuneRegular : FlatBuff<LeveledStatBuff<godoctopus::StatsType::Affinity>, godoctopus::BaseStats> { int32_t level = 0; };

// Special buff runes
struct HitPointBuffRuneSpecial : SpecialScaledBuff<HitPointBuffRuneSpecial, LeveledStatBuff<godoctopus::StatsType::HitPoints>, godoctopus::BaseStats> { int32_t level = 0; };
struct ArmorBuffRuneSpecial : SpecialScaledBuff<ArmorBuffRuneSpecial, LeveledStatBuff<godoctopus::StatsType::Shield>, godoctopus::BaseStats> { int32_t level = 0; };
struct DamageBuffRuneSpecial : SpecialScaledBuff<DamageBuffRuneSpecial, LeveledStatBuff<godoctopus::StatsType::Damage>, godoctopus::BaseStats> { int32_t level = 0; };
struct ReloadBuffRuneSpecial : SpecialScaledBuff<ReloadBuffRuneSpecial, LeveledStatBuff<godoctopus::StatsType::Speed>, godoctopus::BaseStats> { int32_t level = 0; };

// Conditional Special buff runes
struct ConditionalArmorBuffLowLifeRuneTier1 : ConditionalStatBuffLowLifeRune<ConditionalArmorBuffLowLifeRuneTier1, godoctopus::StatsType::Shield, /* under = */ true> {};
struct ConditionalArmorBuffLowLifeRuneTier2 : ConditionalStatBuffLowLifeRune<ConditionalArmorBuffLowLifeRuneTier2, godoctopus::StatsType::Shield, /* under = */ true> {};
struct ConditionalDamageBuffLowLifeRuneTier1 : ConditionalStatBuffLowLifeRune<ConditionalDamageBuffLowLifeRuneTier1, godoctopus::StatsType::Damage, /* under = */ true> {};
struct ConditionalDamageBuffLowLifeRuneTier2 : ConditionalStatBuffLowLifeRune<ConditionalDamageBuffLowLifeRuneTier2, godoctopus::StatsType::Damage, /* under = */ true> {};
struct ConditionalReloadBuffLowLifeRuneTier1 : ConditionalStatBuffLowLifeRune<ConditionalReloadBuffLowLifeRuneTier1, godoctopus::StatsType::Speed, /* under = */ true> {};
struct ConditionalReloadBuffLowLifeRuneTier2 : ConditionalStatBuffLowLifeRune<ConditionalReloadBuffLowLifeRuneTier2, godoctopus::StatsType::Speed, /* under = */ true> {};

struct ConditionalArmorBuffHighLifeRuneTier1 : ConditionalStatBuffLowLifeRune<ConditionalArmorBuffHighLifeRuneTier1, godoctopus::StatsType::Shield, /* under = */ false> {};
struct ConditionalArmorBuffHighLifeRuneTier2 : ConditionalStatBuffLowLifeRune<ConditionalArmorBuffHighLifeRuneTier2, godoctopus::StatsType::Shield, /* under = */ false> {};
struct ConditionalDamageBuffHighLifeRuneTier1 : ConditionalStatBuffLowLifeRune<ConditionalDamageBuffHighLifeRuneTier1, godoctopus::StatsType::Damage, /* under = */ false> {};
struct ConditionalDamageBuffHighLifeRuneTier2 : ConditionalStatBuffLowLifeRune<ConditionalDamageBuffHighLifeRuneTier2, godoctopus::StatsType::Damage, /* under = */ false> {};
struct ConditionalReloadBuffHighLifeRuneTier1 : ConditionalStatBuffLowLifeRune<ConditionalReloadBuffHighLifeRuneTier1, godoctopus::StatsType::Speed, /* under = */ false> {};
struct ConditionalReloadBuffHighLifeRuneTier2 : ConditionalStatBuffLowLifeRune<ConditionalReloadBuffHighLifeRuneTier2, godoctopus::StatsType::Speed, /* under = */ false> {};

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx, custom_step_manager &manager, godot::SmartMMeshLibrary *library = nullptr);

struct ModRuneData {
	int level = 0;
	int flat_buff = 0;
	int base = 0;
	int level_upgrade = 0;
	int upgrade = 0;
	int range = 0;
	int duration_ticks = 0;
};

void mod_rune_based_on_names(flecs::entity e, std::string const &type, std::string const &rune_name, bool add, ModRuneData const &rune_data);
