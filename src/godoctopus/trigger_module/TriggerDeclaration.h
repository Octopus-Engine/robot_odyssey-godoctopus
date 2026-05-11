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
struct AddRuneLoadOnTargetOnAttack { int32_t level = 0; };
struct HealAndConsumeRuneLoadOnHit { int32_t level = 0; };
struct HealAndConsumeRuneLoadOnAttack { int32_t level = 0; };
struct BonusDamageConsumeRuneOnAttack { int32_t level = 0; };
struct BonusDamageRune { int32_t level = 0; };
struct BonusDamageSelfDamage { int32_t level = 0; };
struct AoeDamageOnHit { int32_t level = 0; };
struct AoeDamageOnHitLevel { int32_t level = 0;};
struct AoeDamageConsumeRuneOnHit { int32_t level = 0; };
struct AoeHealOnHit { int32_t level = 0; };
struct AoeHealConsumeRuneOnHit { int32_t level = 0; };
struct AoeHealOnAttack { int32_t level = 0; };
struct AoeDamageSpecial { int32_t level = 0; };
struct AoeDamageConsumeRuneOnAttack { int32_t level = 0; };
struct AoeRuneToEnnemiesOnHit { int32_t level = 0; };
struct AoeRuneToAlliesOnHit { int32_t level = 0; };
struct AddRuneLoadOnHit { int32_t level = 0; };
struct HasLowHpDoubleDamageRune { int32_t level = 0; };
struct HasHighHpBonusDamageRune { int32_t level = 0; };
struct TargetHasHighHpBonusDamageRune { int32_t level = 0; };
struct AoeDamageConsumeRuneOnTargetOnAttack { int32_t level = 0; };
struct BonusDamageConsumeRuneOnTargetOnAttack { int32_t level = 0; };
struct BonusDamageConsumeRuneOnTargetOnAttackTier2 { int32_t level = 0; };
struct BonusDamageConsumeRuneOnTargetOnAttackTier3 { int32_t level = 0; };
struct LifestealRune { int32_t level = 0; };
struct LifestealRuneCore { int32_t level = 0; };
struct LifestealRuneSpecial { int32_t level = 0; };
struct LifestealRuneConsumeRuneOnTarget { int32_t level = 0; };
struct AoeDamageBasedOnHitpointOnDeath { int32_t level = 0; };
struct AoeHealBasedOnHitpointOnDeath { int32_t level = 0; };
struct AoeDamageBasedOnDamageOnDeath { int32_t level = 0; };
struct AoeHealBasedOnDamageOnDeath { int32_t level = 0; };
struct AoeDamageConsumeRuneOnDeath { int32_t level = 0; };
struct AoeHealConsumeRuneOnDeath { int32_t level = 0; };
struct AddAoeRuneLoadToEnemiesOnDeath { int32_t level = 0; };
struct AddAoeRuneLoadToAlliesOnDeath { int32_t level = 0; };

// Tier 2 Runes
struct HealAndConsumeRuneLoadOnAttackTier2 { int32_t level = 0; };
struct AoeDamageConsumeRuneOnHitTier2 { int32_t level = 0; };
struct AoeHealConsumeRuneOnHitTier2 { int32_t level = 0; };
struct AoeDamageConsumeRuneOnAttackTier2 { int32_t level = 0; };
struct TargetHasHighHpBonusDamageRuneTier2 { int32_t level = 0; };
struct AoeDamageConsumeRuneOnTargetOnAttackTier2 { int32_t level = 0; };

// Classic buffs runes
struct HitPointBuffRune : HitPointBuff<10> { int32_t level = 0; };
struct HitPointBuffRuneTier2 : HitPointBuff<50> { int32_t level = 0; };
struct HitPointBuffRuneTier3 : HitPointBuff<100> { int32_t level = 0; };
struct DamageBuffRune : DamageBuff<2> { int32_t level = 0; };
struct DamageBuffRuneTier2 : DamageBuff<10> { int32_t level = 0; };
struct DamageBuffRuneTier3 : DamageBuff<20> { int32_t level = 0; };
struct AttackSpeedBuffRune : AttackSpeedBuff<10> { int32_t level = 0; };
struct AttackSpeedBuffRuneTier2 : AttackSpeedBuff<20> { int32_t level = 0; };
struct AttackSpeedBuffRuneTier3 : AttackSpeedBuff<50> { int32_t level = 0; };

// Regular buffs runes
struct DamageBuffRuneRegular : LeveledBuff<LeveledDamageBuff<2, 1>, octopus::Attack> {};
struct HitPointBuffRuneRegular : LeveledBuff<LeveledHitPointBuff<10, 5>, octopus::HitPoint, octopus::HitPointMax> {};
struct ReloadBuffRuneRegular : LeveledBuff<LeveledAttackSpeedBuff<5, 1>, octopus::Attack> {};
struct ArmorBuffRuneRegular : LeveledBuff<LeveledArmorBuff<10, 5>, octopus::Armor> {};
struct SpecialBuffRuneRegular : LeveledBuff<LeveledSpecialBuff<2, 1>, Special> {};
struct AffinityBuffRuneRegular : LeveledBuff<LeveledAffinityBuff<2, 1>, Special> {};

// Core buff runes
struct HitPointBuffRuneCore : SpecialScaledBuff<HitPointBuffRuneCore, LeveledHitPointBuff<20, 1>, octopus::HitPoint, octopus::HitPointMax> { int32_t level = 0; };
struct ArmorBuffRuneCore : SpecialScaledBuff<ArmorBuffRuneCore, LeveledArmorBuff<20, 1>, octopus::Armor> { int32_t level = 0; };
struct DamageBuffRuneCore : SpecialScaledBuff<DamageBuffRuneCore, LeveledDamageBuff<5, 1>, octopus::Attack> { int32_t level = 0; };
struct ReloadBuffRuneCore : SpecialScaledBuff<ReloadBuffRuneCore, LeveledAttackSpeedBuff<10, 1>, octopus::Attack> { int32_t level = 0; };

// Special buff runes
struct HitPointBuffRuneSpecial : SpecialScaledBuff<HitPointBuffRuneSpecial, LeveledHitPointBuff<50, 2>, octopus::HitPoint, octopus::HitPointMax> { int32_t level = 0; };
struct ArmorBuffRuneSpecial : SpecialScaledBuff<ArmorBuffRuneSpecial, LeveledArmorBuff<40, 2>, octopus::Armor> { int32_t level = 0; };
struct DamageBuffRuneSpecial : SpecialScaledBuff<DamageBuffRuneSpecial, LeveledDamageBuff<10, 2>, octopus::Attack> { int32_t level = 0; };
struct ReloadBuffRuneSpecial : SpecialScaledBuff<ReloadBuffRuneSpecial, LeveledAttackSpeedBuff<20, 2>, octopus::Attack> { int32_t level = 0; };

// Conditional Special buff runes
using ConditionalArmorBuffLowLifeRuneTier1 = ConditionalArmorBuffLowLifeRune</* under = */ true, /* percent = */ 25, /* armor_x10_base = */ 20, /* armor_x10_per_special = */ 2>;
using ConditionalArmorBuffLowLifeRuneTier2 = ConditionalArmorBuffLowLifeRune</* under = */ true, /* percent = */ 25, /* armor_x10_base = */ 50, /* armor_x10_per_special = */ 5>;
using ConditionalDamageBuffLowLifeRuneTier1 = ConditionalDamageBuffLowLifeRune</* under = */ true, /* percent = */ 25, /* damage_base = */ 8, /* damage_per_special = */ 1>;
using ConditionalDamageBuffLowLifeRuneTier2 = ConditionalDamageBuffLowLifeRune</* under = */ true, /* percent = */ 25, /* damage_base = */ 15, /* damage_per_special = */ 2>;
using ConditionalReloadBuffLowLifeRuneTier1 = ConditionalReloadBuffLowLifeRune</* under = */ true, /* percent = */ 25, /* reload_x10_base = */ 20, /* reload_x10_per_special = */ 2>;
using ConditionalReloadBuffLowLifeRuneTier2 = ConditionalReloadBuffLowLifeRune</* under = */ true, /* percent = */ 25, /* reload_x10_base = */ 40, /* reload_x10_per_special = */ 4>;

using ConditionalArmorBuffHighLifeRuneTier1 = ConditionalArmorBuffLowLifeRune</* under = */ false, /* percent = */ 80, /* armor_x10_base = */ 10, /* armor_x10_per_special = */ 1>;
using ConditionalArmorBuffHighLifeRuneTier2 = ConditionalArmorBuffLowLifeRune</* under = */ false, /* percent = */ 80, /* armor_x10_base = */ 30, /* armor_x10_per_special = */ 3>;
using ConditionalDamageBuffHighLifeRuneTier1 = ConditionalDamageBuffLowLifeRune</* under = */ false, /* percent = */ 80, /* damage_base = */ 4, /* damage_per_special = */ 1>;
using ConditionalDamageBuffHighLifeRuneTier2 = ConditionalDamageBuffLowLifeRune</* under = */ false, /* percent = */ 80, /* damage_base = */ 8, /* damage_per_special = */ 2>;
using ConditionalReloadBuffHighLifeRuneTier1 = ConditionalReloadBuffLowLifeRune</* under = */ false, /* percent = */ 80, /* reload_x10_base = */ 10, /* reload_x10_per_special = */ 2>;
using ConditionalReloadBuffHighLifeRuneTier2 = ConditionalReloadBuffLowLifeRune</* under = */ false, /* percent = */ 80, /* reload_x10_base = */ 20, /* reload_x10_per_special = */ 4>;

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx, custom_step_manager &manager, godot::SmartMMeshLibrary *library = nullptr);

void mod_rune_based_on_names(flecs::entity e, std::string const &type, std::string const &rune_name, bool add, int level);
