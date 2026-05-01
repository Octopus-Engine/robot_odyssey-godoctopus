#include "TriggerDeclaration.h"

#include "flecs.h"
#include "core/variant/variant.h"
#include "core/string/print_string.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"

#include "godoctopus/components/types/Types.h"
#include "godoctopus/trigger_module/TemporaryBuffTriggerDeclaration.h"
#include "godoctopus/trigger_module/AoePulseRune.h"

template<bool Level, typename RuneType, typename UnitType, typename... ComponentType>
typename std::enable_if<Level, void>::type mod_rune(flecs::entity e, bool add, int level)
{
	e.world().defer_suspend();
	e.remove<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>();
	if(add) {
		RuneType rune;
		rune.level = level;
		e.set<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>({std::move(rune)});
	}
	e.world().defer_resume();
}

template<bool Level, typename RuneType, typename UnitType, typename... ComponentType>
typename std::enable_if<!Level, void>::type mod_rune(flecs::entity e, bool add, int level)
{
	e.world().defer_suspend();
	e.remove<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>();
	if(add) {
		e.set<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>({{level}});
	}
	e.world().defer_resume();
}

template<bool Level, typename RuneType, typename... ComponentType>
struct ModRune {
	flecs::entity e;
	bool add;
	std::string const &type;
	int level;

	template<typename BotType>
	void operator()() const {
		if(type == BotType::naming()) {
			mod_rune<Level, RuneType, BotType, ComponentType...>(e, add, level);
		}
	}
};

template<bool Level, typename RuneType, typename... ComponentType>
void mod_rune_type(flecs::entity e, bool add, std::string const &type, int level=0)
{
	for_each_bot_type(ModRune<Level, RuneType, ComponentType...>{e, add, type, level});
}

void mod_rune_based_on_names(flecs::entity e, std::string const &type, std::string const &rune_name, bool add, int level)
{
	if(rune_name == "AddRuneLoadOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<AddRuneLoadOnAttack>>(e, add, type, level);
	}
	else if(rune_name == "AddRuneLoadOnTargetOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<AddRuneLoadOnTargetOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "HealAndConsumeRuneLoadOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<HealAndConsumeRuneLoadOnHit>>(e, add, type, level);
	}
	else if (rune_name == "HealAndConsumeRuneLoadOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<HealAndConsumeRuneLoadOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "HealAndConsumeRuneLoadOnAttackTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<HealAndConsumeRuneLoadOnAttackTier2>>(e, add, type, level);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "BonusDamageRune") {
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageRune>>(e, add, type, level);
	}
	else if (rune_name == "BonusDamageSelfDamage") {
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageSelfDamage>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageOnHit>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageOnHitLevel") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageOnHitLevel>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnHit>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnHitTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnHitTier2>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealOnHit>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealConsumeRuneOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealConsumeRuneOnHit>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealConsumeRuneOnHitTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealConsumeRuneOnHitTier2>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageSpecial") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageSpecial>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnAttackTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnAttackTier2>>(e, add, type, level);
	}
	else if (rune_name == "AoeRuneToEnnemiesOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeRuneToEnnemiesOnHit>>(e, add, type, level);
	}
	else if (rune_name == "AoeRuneToAlliesOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeRuneToAlliesOnHit>>(e, add, type, level);
	}
	else if (rune_name == "AddRuneLoadOnHit") {
		mod_rune_type<false, octopus::BuffAddComponent<AddRuneLoadOnHit>>(e, add, type, level);
	}
	else if (rune_name == "HasLowHpDoubleDamageRune") {
		mod_rune_type<false, octopus::BuffAddComponent<HasLowHpDoubleDamageRune>>(e, add, type, level);
	}
	else if (rune_name == "HasHighHpBonusDamageRune") {
		mod_rune_type<false, octopus::BuffAddComponent<HasHighHpBonusDamageRune>>(e, add, type, level);
	}
	else if (rune_name == "HitPointBuffRune") {
		mod_rune_type<true, HitPointBuffRune, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "HitPointBuffRuneTier2") {
		mod_rune_type<true, HitPointBuffRuneTier2, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "HitPointBuffRuneTier3") {
		mod_rune_type<true, HitPointBuffRuneTier3, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "DamageBuffRune") {
		mod_rune_type<true, DamageBuffRune, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "DamageBuffRuneTier2") {
		mod_rune_type<true, DamageBuffRuneTier2, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "DamageBuffRuneTier3") {
		mod_rune_type<true, DamageBuffRuneTier3, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "AttackSpeedBuffRune") {
		mod_rune_type<true, AttackSpeedBuffRune, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "AttackSpeedBuffRuneTier2") {
		mod_rune_type<true, AttackSpeedBuffRuneTier2, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "AttackSpeedBuffRuneTier3") {
		mod_rune_type<true, AttackSpeedBuffRuneTier3, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "HitPointBuffRuneRegular") {
		mod_rune_type<true, HitPointBuffRuneRegular, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ArmorBuffRuneRegular") {
		mod_rune_type<true, ArmorBuffRuneRegular, octopus::Armor>(e, add, type, level);
	}
	else if (rune_name == "DamageBuffRuneRegular") {
		mod_rune_type<true, DamageBuffRuneRegular, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "ReloadBuffRuneRegular") {
		mod_rune_type<true, ReloadBuffRuneRegular, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "SpecialBuffRuneRegular") {
		mod_rune_type<true, SpecialBuffRuneRegular, Special>(e, add, type, level);
	}
	else if (rune_name == "AffinityBuffRuneRegular") {
		mod_rune_type<true, AffinityBuffRuneRegular, Special>(e, add, type, level);
	}
	else if (rune_name == "HitPointBuffRuneCore") {
		mod_rune_type<true, HitPointBuffRuneCore, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ArmorBuffRuneCore") {
		mod_rune_type<true, ArmorBuffRuneCore, octopus::Armor>(e, add, type, level);
	}
	else if (rune_name == "DamageBuffRuneCore") {
		mod_rune_type<true, DamageBuffRuneCore, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "ReloadBuffRuneCore") {
		mod_rune_type<true, ReloadBuffRuneCore, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "HitPointBuffRuneSpecial") {
		mod_rune_type<true, HitPointBuffRuneSpecial, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ArmorBuffRuneSpecial") {
		mod_rune_type<true, ArmorBuffRuneSpecial, octopus::Armor>(e, add, type, level);
	}
	else if (rune_name == "DamageBuffRuneSpecial") {
		mod_rune_type<true, DamageBuffRuneSpecial, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "ReloadBuffRuneSpecial") {
		mod_rune_type<true, ReloadBuffRuneSpecial, octopus::Attack>(e, add, type, level);
	}
	else if (rune_name == "TargetHasHighHpBonusDamageRune") {
		mod_rune_type<false, octopus::BuffAddComponent<TargetHasHighHpBonusDamageRune>>(e, add, type, level);
	}
	else if (rune_name == "TargetHasHighHpBonusDamageRuneTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<TargetHasHighHpBonusDamageRuneTier2>>(e, add, type, level);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnTargetOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnTargetOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnTargetOnAttackTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnTargetOnAttackTier2>>(e, add, type, level);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnTargetOnAttackTier3") {
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnTargetOnAttackTier3>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnTargetOnAttack") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnTargetOnAttack>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnTargetOnAttackTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnTargetOnAttackTier2>>(e, add, type, level);
	}
	else if (rune_name == "LifestealRuneCore") {
		mod_rune_type<false, octopus::BuffAddComponent<LifestealRuneCore>>(e, add, type, level);
	}
	else if (rune_name == "LifestealRuneSpecial") {
		mod_rune_type<false, octopus::BuffAddComponent<LifestealRuneSpecial>>(e, add, type, level);
	}
	else if (rune_name == "LifestealRune") {
		mod_rune_type<false, octopus::BuffAddComponent<LifestealRune>>(e, add, type, level);
	}
	else if (rune_name == "LifestealRuneConsumeRuneOnTarget") {
		mod_rune_type<false, octopus::BuffAddComponent<LifestealRuneConsumeRuneOnTarget>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageBasedOnHitpointOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageBasedOnHitpointOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealBasedOnHitpointOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealBasedOnHitpointOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageBasedOnDamageOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageBasedOnDamageOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealBasedOnDamageOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealBasedOnDamageOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealConsumeRuneOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealConsumeRuneOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AddAoeRuneLoadToEnemiesOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AddAoeRuneLoadToEnemiesOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AddAoeRuneLoadToAlliesOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AddAoeRuneLoadToAlliesOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseHealBasedOnHitpoint") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseHealBasedOnHitpoint>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseHealBasedOnHitpointTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseHealBasedOnHitpointTier2>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseHealBasedOnDamage") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseHealBasedOnDamage>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseHealBasedOnDamageTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseHealBasedOnDamageTier2>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseDamageBasedOnHitpoint") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseDamageBasedOnHitpoint>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseDamageBasedOnHitpointTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseDamageBasedOnHitpointTier2>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseDamageBasedOnDamage") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseDamageBasedOnDamage>>(e, add, type, level);
	}
	else if (rune_name == "AoePulseDamageBasedOnDamageTier2") {
		mod_rune_type<false, octopus::BuffAddComponent<AoePulseDamageBasedOnDamageTier2>>(e, add, type, level);
	}
	else if (rune_name == "ApplyHealthBuffOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyHealthBuffOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyArmorBuffOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyArmorBuffOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyDamageBuffOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyDamageBuffOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyAttackSpeedBuffOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyAttackSpeedBuffOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyHealthBuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyHealthBuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyArmorBuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyArmorBuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyDamageBuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyDamageBuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyAttackSpeedBuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyAttackSpeedBuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyArmorDebuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyArmorDebuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyDamageDebuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyDamageDebuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ApplyAttackSpeedDebuffAreaOnRuneLoad") {
		mod_rune_type<false, octopus::BuffAddComponent<ApplyAttackSpeedDebuffAreaOnRuneLoad>>(e, add, type, level);
	}
	else if (rune_name == "ConditionalArmorBuffLowLifeRuneTier1") {
		mod_rune_type<true, ConditionalArmorBuffLowLifeRuneTier1, octopus::Armor, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ConditionalArmorBuffLowLifeRuneTier2") {
		mod_rune_type<true, ConditionalArmorBuffLowLifeRuneTier2, octopus::Armor, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ConditionalDamageBuffLowLifeRuneTier1") {
		mod_rune_type<true, ConditionalDamageBuffLowLifeRuneTier1, octopus::Attack, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ConditionalDamageBuffLowLifeRuneTier2") {
		mod_rune_type<true, ConditionalDamageBuffLowLifeRuneTier2, octopus::Attack, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ConditionalReloadBuffLowLifeRuneTier1") {
		mod_rune_type<true, ConditionalReloadBuffLowLifeRuneTier1, octopus::Attack, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else if (rune_name == "ConditionalReloadBuffLowLifeRuneTier2") {
		mod_rune_type<true, ConditionalReloadBuffLowLifeRuneTier2, octopus::Attack, octopus::HitPoint, octopus::HitPointMax>(e, add, type, level);
	}
	else {
		print_line("mod_rune_based_on_names: Unknown rune name ", rune_name.c_str());
	}
}
