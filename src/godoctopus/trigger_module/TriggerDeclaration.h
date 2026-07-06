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
struct DamageBuffRuneRegular : FlatBuff<LeveledDamageBuff, octopus::Attack> { int32_t level = 0; };
struct HitPointBuffRuneRegular : FlatBuff<LeveledHitPointBuff, octopus::HitPoint, octopus::HitPointMax> { int32_t level = 0; };
struct ReloadBuffRuneRegular : FlatBuff<LeveledAttackSpeedBuff, octopus::Attack> { int32_t level = 0; };
struct ArmorBuffRuneRegular : FlatBuff<LeveledArmorBuff, octopus::Armor> { int32_t level = 0; };
struct SpecialBuffRuneRegular : FlatBuff<LeveledSpecialBuff, Special> { int32_t level = 0; };
struct AffinityBuffRuneRegular : FlatBuff<LeveledAffinityBuff, Special> { int32_t level = 0; };

// Core buff runes
struct HitPointBuffRuneCore : SpecialScaledBuff<HitPointBuffRuneCore, LeveledHitPointBuff, octopus::HitPoint, octopus::HitPointMax> { int32_t level = 0; };
struct ArmorBuffRuneCore : SpecialScaledBuff<ArmorBuffRuneCore, LeveledArmorBuff, octopus::Armor> { int32_t level = 0; };
struct DamageBuffRuneCore : SpecialScaledBuff<DamageBuffRuneCore, LeveledDamageBuff, octopus::Attack> { int32_t level = 0; };
struct ReloadBuffRuneCore : SpecialScaledBuff<ReloadBuffRuneCore, LeveledAttackSpeedBuff, octopus::Attack> { int32_t level = 0; };

// Special buff runes
struct HitPointBuffRuneSpecial : SpecialScaledBuff<HitPointBuffRuneSpecial, LeveledHitPointBuff, octopus::HitPoint, octopus::HitPointMax> { int32_t level = 0; };
struct ArmorBuffRuneSpecial : SpecialScaledBuff<ArmorBuffRuneSpecial, LeveledArmorBuff, octopus::Armor> { int32_t level = 0; };
struct DamageBuffRuneSpecial : SpecialScaledBuff<DamageBuffRuneSpecial, LeveledDamageBuff, octopus::Attack> { int32_t level = 0; };
struct ReloadBuffRuneSpecial : SpecialScaledBuff<ReloadBuffRuneSpecial, LeveledAttackSpeedBuff, octopus::Attack> { int32_t level = 0; };

// Conditional Special buff runes
struct ConditionalArmorBuffLowLifeRuneTier1 : ConditionalArmorBuffLowLifeRune<ConditionalArmorBuffLowLifeRuneTier1, /* under = */ true> {};
struct ConditionalArmorBuffLowLifeRuneTier2 : ConditionalArmorBuffLowLifeRune<ConditionalArmorBuffLowLifeRuneTier2, /* under = */ true> {};
struct ConditionalDamageBuffLowLifeRuneTier1 : ConditionalDamageBuffLowLifeRune<ConditionalDamageBuffLowLifeRuneTier1, /* under = */ true> {};
struct ConditionalDamageBuffLowLifeRuneTier2 : ConditionalDamageBuffLowLifeRune<ConditionalDamageBuffLowLifeRuneTier2, /* under = */ true> {};
struct ConditionalReloadBuffLowLifeRuneTier1 : ConditionalReloadBuffLowLifeRune<ConditionalReloadBuffLowLifeRuneTier1, /* under = */ true> {};
struct ConditionalReloadBuffLowLifeRuneTier2 : ConditionalReloadBuffLowLifeRune<ConditionalReloadBuffLowLifeRuneTier2, /* under = */ true> {};

struct ConditionalArmorBuffHighLifeRuneTier1 : ConditionalArmorBuffLowLifeRune<ConditionalArmorBuffHighLifeRuneTier1, /* under = */ false> {};
struct ConditionalArmorBuffHighLifeRuneTier2 : ConditionalArmorBuffLowLifeRune<ConditionalArmorBuffHighLifeRuneTier2, /* under = */ false> {};
struct ConditionalDamageBuffHighLifeRuneTier1 : ConditionalDamageBuffLowLifeRune<ConditionalDamageBuffHighLifeRuneTier1, /* under = */ false> {};
struct ConditionalDamageBuffHighLifeRuneTier2 : ConditionalDamageBuffLowLifeRune<ConditionalDamageBuffHighLifeRuneTier2, /* under = */ false> {};
struct ConditionalReloadBuffHighLifeRuneTier1 : ConditionalReloadBuffLowLifeRune<ConditionalReloadBuffHighLifeRuneTier1, /* under = */ false> {};
struct ConditionalReloadBuffHighLifeRuneTier2 : ConditionalReloadBuffLowLifeRune<ConditionalReloadBuffHighLifeRuneTier2, /* under = */ false> {};

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
