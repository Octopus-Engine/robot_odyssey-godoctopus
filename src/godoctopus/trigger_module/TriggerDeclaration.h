#pragma once

#include "flecs.h"
#include "octopus/world/position/PositionContext.hh"

#include "godoctopus/trigger_module/buffs/ArmorBuff.h"
#include "godoctopus/trigger_module/buffs/AttackSpeedBuff.h"
#include "godoctopus/trigger_module/buffs/DamageBuff.h"
#include "godoctopus/trigger_module/buffs/HitPointBuff.h"
#include "godoctopus/trigger_module/buffs/SpecialBuff.h"
#include "godoctopus/trigger_module/buffs/ScalingBuff.h"

struct AddRuneLoadOnAttack { int32_t level = 0; };
struct AddRuneLoadOnTargetOnAttack { int32_t level = 0; };
struct HealAndConsumeRuneLoadOnAttack { int32_t level = 0; };
struct DoubleDamageRune { int32_t level = 0; };
struct BonusDamageSelfDamage { int32_t level = 0; };
struct AoeDamageOnHit { int32_t level = 0; };
struct AoeDamageOnHitLevel { int32_t level = 0;};
struct AoeDamageConsumeRuneOnHit { int32_t level = 0; };
struct AoeHealOnHit { int32_t level = 0; };
struct AoeHealConsumeRuneOnHit { int32_t level = 0; };
struct AoeHealOnAttack { int32_t level = 0; };
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
struct LifestealRuneConsumeRuneOnTarget { int32_t level = 0; };
struct AoeDamageOnDeath { int32_t level = 0; };
struct AoeHealOnDeath { int32_t level = 0; };
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
struct HitPointBuffRuneRegular : LeveledBuff<LeveledHitPointBuff<10, 5>, octopus::HitPoint, octopus::HitPointMax> {};
struct ArmorBuffRuneRegular : LeveledBuff<LeveledArmorBuff<10, 5>, octopus::Armor> {};
struct DamageBuffRuneRegular : LeveledBuff<LeveledDamageBuff<2, 1>, octopus::Attack> {};
struct ReloadBuffRuneRegular : LeveledBuff<LeveledAttackSpeedBuff<5, 5>, octopus::Attack> {};
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

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx);

void mod_rune_based_on_names(flecs::entity e, std::string const &type, std::string const &rune_name, bool add, int level);
