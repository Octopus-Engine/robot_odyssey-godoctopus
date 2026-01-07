#include "TriggerDeclaration.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"

#include "godoctopus/components/types/Types.h"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/conditions/AlwaysCondition.h"
#include "godoctopus/trigger_module/conditions/HitpointCondition.h"
#include "godoctopus/trigger_module/conditions/RuneCondition.h"
#include "godoctopus/trigger_module/events/attack/FactorDamageEvent.h"
#include "godoctopus/trigger_module/events/attack/LifestealEvent.h"
#include "godoctopus/trigger_module/events/HitpointEvent.h"
#include "godoctopus/trigger_module/events/RuneEvent.h"
#include "godoctopus/trigger_module/events/UtilsEvent.h"
#include "godoctopus/trigger_module/trigger_systems/AttackTriggerSystem.h"
#include "godoctopus/trigger_module/trigger_systems/LifeTriggerSystem.h"

template<typename BuffType, typename... ComponentType>
struct BuffDeclarer {
	flecs::world &ecs;
	bool add_debuff_all_system;

	template<typename BotType>
	void operator()() const {
		octopus::declare_player_buff_systems<BotType, BuffType, ComponentType...>(ecs, add_debuff_all_system);
	}
};

template<typename BuffType, typename... ComponentType>
void declare_player_buff_systems_all_units(flecs::world &ecs, bool add_debuff_all_system)
{
	for_each_bot_type(BuffDeclarer<BuffType, ComponentType...>{ecs, add_debuff_all_system});
}

template<typename Trigger>
void declare_trigger_buff(flecs::world &ecs)
{
	// component declaration
	ecs.component<Trigger>()
		.member("level", &Trigger::level)
	;
	ecs.component<octopus::BuffAddComponent<Trigger>>()
		.member("placeholder", &octopus::BuffAddComponent<Trigger>::placeholder)
	;

	declare_player_buff_systems_all_units<typename octopus::BuffAddComponent<Trigger>>(ecs, false);
}

template<typename BuffType, typename... ComponentType>
void declare_classic_buff(flecs::world &ecs)
{
	// component declaration
	ecs.component<BuffType>()
		.member("quantity", &BuffType::quantity);

	declare_player_buff_systems_all_units<BuffType, ComponentType...>(ecs, true);
}

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx)
{
	declare_trigger_life_system(ecs);
	declare_trigger_attack_system(ecs);

	declare_trigger_system<AddRuneLoadOnAttack, trigger_module::DamageDealt, AlwaysCondition, RuneEvent<DefaultRune, 1>>(ecs);
	declare_attack_trigger_system<AddRuneLoadOnTargetOnAttack, trigger_module::DamageDealt, AlwaysCondition, TargetEvent<RuneEvent<DefaultRune, 1>>>(ecs);

	declare_trigger_system<HealAndConsumeRuneLoadOnAttack, trigger_module::Attack, RuneCondition<1, DefaultRune>, HitpointEvent<5,1>>(ecs);
	declare_trigger_system<HealAndConsumeRuneLoadOnAttackTier2, trigger_module::Attack, RuneCondition<2, DefaultRune>, HitpointEvent<10,2>>(ecs);

	declare_attack_trigger_system<DoubleDamageRune, trigger_module::DamageDealt, AlwaysCondition, FactorDamageEvent<20, 5>>(ecs);
	declare_attack_trigger_system<HasLowHpDoubleDamageRune, trigger_module::DamageDealt, HitPointUnderPercentCondition<25>, FactorDamageEvent<100, 25>>(ecs);
	declare_attack_trigger_system<HasHighHpBonusDamageRune, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<50, 25>>(ecs);

	declare_attack_trigger_system<BonusDamageSelfDamage, trigger_module::DamageDealt, HitPointCostCondition<2>, FactorDamageEvent<150, 50>>(ecs);

	declare_area_trigger_system<AoeDamageOnHit, trigger_module::DamageReceived, AlwaysCondition, DamageAreaEvent<1, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageOnHitLevel, trigger_module::DamageReceived, AlwaysCondition, DamageAreaEvent<1, 5, 1>>(ecs, ctx);

	declare_area_trigger_system<AoeDamageConsumeRuneOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageConsumeRuneOnHitTier2, trigger_module::DamageReceived, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	declare_area_trigger_system<AoeHealOnHit, trigger_module::DamageReceived, AlwaysCondition, HealAreaEvent<2, 5, 1>>(ecs, ctx);

	declare_area_trigger_system<AoeHealConsumeRuneOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, HealAreaEvent<4, 5, 2>>(ecs, ctx);
	declare_area_trigger_system<AoeHealConsumeRuneOnHitTier2, trigger_module::DamageReceived, RuneCondition<2, DefaultRune>, HealAreaEvent<8, 5, 4>>(ecs, ctx);

	declare_area_trigger_system<AoeHealOnAttack, trigger_module::DamageDealt, AlwaysCondition, HealAreaEvent<8, 2, 4>>(ecs, ctx);

	declare_attack_area_trigger_system<AoeDamageConsumeRuneOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	declare_attack_area_trigger_system<AoeDamageConsumeRuneOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	declare_area_trigger_system<AoeRuneToEnnemiesOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, false>>(ecs, ctx);

	declare_area_trigger_system<AoeRuneToAlliesOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, true>>(ecs, ctx);

	declare_trigger_system<AddRuneLoadOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneEvent<DefaultRune, 1>>(ecs);

	declare_attack_trigger_target_condition_system<TargetHasHighHpBonusDamageRune, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<50, 10>>(ecs);
	declare_attack_trigger_target_condition_system<TargetHasHighHpBonusDamageRuneTier2, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<100, 20>>(ecs);
	declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, FactorDamageEvent<100, 20>>(ecs);
	declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, FactorDamageEvent<200, 50>>(ecs);
	declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttackTier3, trigger_module::DamageDealt, RuneCondition<3, DefaultRune>, FactorDamageEvent<300, 100>>(ecs);

	declare_attack_area_trigger_target_condition_system<AoeDamageConsumeRuneOnTargetOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	declare_attack_area_trigger_target_condition_system<AoeDamageConsumeRuneOnTargetOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	declare_attack_trigger_system<LifestealRune, trigger_module::DamageDealt, AlwaysCondition, LifestealEvent<5, 1> >(ecs);
	declare_attack_trigger_target_condition_system<LifestealRuneConsumeRuneOnTarget, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, LifestealEvent<20, 4> >(ecs);

	declare_area_trigger_system<AoeDamageOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEvent<4, 5, 4>>(ecs, ctx);
	declare_area_trigger_system<AoeHealOnDeath, trigger_module::Death, AlwaysCondition, HealAreaEvent<10, 5, 5>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageConsumeRuneOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEvent<8, 5, 8>>(ecs, ctx);
	declare_area_trigger_system<AoeHealConsumeRuneOnDeath, trigger_module::Death, RuneCondition<1, DefaultRune>, HealAreaEvent<15, 5, 10>>(ecs, ctx);
	declare_area_trigger_system<AddAoeRuneLoadToEnemiesOnDeath, trigger_module::Death, RuneCondition<1, DefaultRune>, RuneAreaEvent<DefaultRune, 1, 5, false>>(ecs, ctx);
	declare_area_trigger_system<AddAoeRuneLoadToAlliesOnDeath, trigger_module::Death, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, true>>(ecs, ctx);

	// declare all trigger buff systems
	declare_trigger_buff<AddRuneLoadOnAttack>(ecs);
	declare_trigger_buff<AddRuneLoadOnTargetOnAttack>(ecs);
	declare_trigger_buff<HealAndConsumeRuneLoadOnAttack>(ecs);
	declare_trigger_buff<HealAndConsumeRuneLoadOnAttackTier2>(ecs);
	declare_trigger_buff<DoubleDamageRune>(ecs);
	declare_trigger_buff<HasLowHpDoubleDamageRune>(ecs);
	declare_trigger_buff<HasHighHpBonusDamageRune>(ecs);
	declare_trigger_buff<BonusDamageSelfDamage>(ecs);
	declare_trigger_buff<AoeDamageOnHit>(ecs);
	declare_trigger_buff<AoeDamageOnHitLevel>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnHit>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnHitTier2>(ecs);
	declare_trigger_buff<AoeHealOnHit>(ecs);
	declare_trigger_buff<AoeHealConsumeRuneOnHit>(ecs);
	declare_trigger_buff<AoeHealConsumeRuneOnHitTier2>(ecs);
	declare_trigger_buff<AoeHealOnAttack>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnAttack>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnAttackTier2>(ecs);
	declare_trigger_buff<AoeRuneToEnnemiesOnHit>(ecs);
	declare_trigger_buff<AoeRuneToAlliesOnHit>(ecs);
	declare_trigger_buff<AddRuneLoadOnHit>(ecs);
	declare_trigger_buff<TargetHasHighHpBonusDamageRune>(ecs);
	declare_trigger_buff<TargetHasHighHpBonusDamageRuneTier2>(ecs);
	declare_trigger_buff<BonusDamageConsumeRuneOnTargetOnAttack>(ecs);
	declare_trigger_buff<BonusDamageConsumeRuneOnTargetOnAttackTier2>(ecs);
	declare_trigger_buff<BonusDamageConsumeRuneOnTargetOnAttackTier3>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnTargetOnAttack>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnTargetOnAttackTier2>(ecs);
	declare_trigger_buff<LifestealRune>(ecs);
	declare_trigger_buff<LifestealRuneConsumeRuneOnTarget>(ecs);
	declare_trigger_buff<AoeDamageOnDeath>(ecs);
	declare_trigger_buff<AoeHealOnDeath>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnDeath>(ecs);
	declare_trigger_buff<AoeHealConsumeRuneOnDeath>(ecs);
	declare_trigger_buff<AddAoeRuneLoadToEnemiesOnDeath>(ecs);
	declare_trigger_buff<AddAoeRuneLoadToAlliesOnDeath>(ecs);

	// declare all classic buffs
	declare_classic_buff<HitPointBuffRune, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_classic_buff<HitPointBuffRuneTier2, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_classic_buff<HitPointBuffRuneTier3, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_classic_buff<DamageBuffRune, octopus::Attack>(ecs);
	declare_classic_buff<DamageBuffRuneTier2, octopus::Attack>(ecs);
	declare_classic_buff<DamageBuffRuneTier3, octopus::Attack>(ecs);
	declare_classic_buff<AttackSpeedBuffRune, octopus::Attack>(ecs);
	declare_classic_buff<AttackSpeedBuffRuneTier2, octopus::Attack>(ecs);
	declare_classic_buff<AttackSpeedBuffRuneTier3, octopus::Attack>(ecs);
}

template<bool Level, typename RuneType, typename UnitType, typename... ComponentType>
typename std::enable_if<Level, void>::type mod_rune(flecs::entity e, bool add, int level)
{
	if(add)
	{
		e.set<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>({{level}});
	}
	else
	{
		e.remove<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>();
	}
}

template<bool Level, typename RuneType, typename UnitType, typename... ComponentType>
typename std::enable_if<!Level, void>::type mod_rune(flecs::entity e, bool add, int)
{
	if(add)
	{
		e.set<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>({});
	}
	else
	{
		e.remove<octopus::PlayerBuff<UnitType, RuneType, ComponentType...>>();
	}
}

template<bool Level, typename RuneType, typename... ComponentType>
void mod_rune_type(flecs::entity e, bool add, std::string const &type, int level=0)
{
	if(type == HealBot::naming())
	{
		mod_rune<Level, RuneType, HealBot, ComponentType...>(e, add, level);
	}
}

void mod_rune_based_on_names(flecs::entity e, std::string const &type, std::string const &rune_name, bool add, int level)
{
	if(rune_name == "AddRuneLoadOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AddRuneLoadOnAttack>>(e, add, type);
	}
	else if(rune_name == "AddRuneLoadOnTargetOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AddRuneLoadOnTargetOnAttack>>(e, add, type);
	}
	else if (rune_name == "HealAndConsumeRuneLoadOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<HealAndConsumeRuneLoadOnAttack>>(e, add, type);
	}
	else if (rune_name == "HealAndConsumeRuneLoadOnAttackTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<HealAndConsumeRuneLoadOnAttackTier2>>(e, add, type);
	}
	else if (rune_name == "DoubleDamageRune")
	{
		mod_rune_type<false, octopus::BuffAddComponent<DoubleDamageRune>>(e, add, type);
	}
	else if (rune_name == "BonusDamageSelfDamage")
	{
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageSelfDamage>>(e, add, type);
	}
	else if (rune_name == "AoeDamageOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageOnHit>>(e, add, type);
	}
	else if (rune_name == "AoeDamageOnHitLevel")
	{
		mod_rune_type<true, octopus::BuffAddComponent<AoeDamageOnHitLevel>>(e, add, type, level);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnHit>>(e, add, type);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnHitTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnHitTier2>>(e, add, type);
	}
	else if (rune_name == "AoeHealOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealOnHit>>(e, add, type);
	}
	else if (rune_name == "AoeHealConsumeRuneOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealConsumeRuneOnHit>>(e, add, type);
	}
	else if (rune_name == "AoeHealConsumeRuneOnHitTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealConsumeRuneOnHitTier2>>(e, add, type);
	}
	else if (rune_name == "AoeHealOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealOnAttack>>(e, add, type);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnAttack>>(e, add, type);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnAttackTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnAttackTier2>>(e, add, type);
	}
	else if (rune_name == "AoeRuneToEnnemiesOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeRuneToEnnemiesOnHit>>(e, add, type);
	}
	else if (rune_name == "AoeRuneToAlliesOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeRuneToAlliesOnHit>>(e, add, type);
	}
	else if (rune_name == "AddRuneLoadOnHit")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AddRuneLoadOnHit>>(e, add, type);
	}
	else if (rune_name == "HasLowHpDoubleDamageRune")
	{
		mod_rune_type<false, octopus::BuffAddComponent<HasLowHpDoubleDamageRune>>(e, add, type);
	}
	else if (rune_name == "HasHighHpBonusDamageRune")
	{
		mod_rune_type<false, octopus::BuffAddComponent<HasHighHpBonusDamageRune>>(e, add, type);
	}
	else if (rune_name == "HitPointBuffRune")
	{
		mod_rune_type<false, HitPointBuffRune, octopus::HitPoint, octopus::HitPointMax>(e, add, type);
	}
	else if (rune_name == "HitPointBuffRuneTier2")
	{
		mod_rune_type<false, HitPointBuffRuneTier2, octopus::HitPoint, octopus::HitPointMax>(e, add, type);
	}
	else if (rune_name == "HitPointBuffRuneTier3")
	{
		mod_rune_type<false, HitPointBuffRuneTier3, octopus::HitPoint, octopus::HitPointMax>(e, add, type);
	}
	else if (rune_name == "DamageBuffRune")
	{
		mod_rune_type<false, DamageBuffRune, octopus::Attack>(e, add, type);
	}
	else if (rune_name == "DamageBuffRuneTier2")
	{
		mod_rune_type<false, DamageBuffRuneTier2, octopus::Attack>(e, add, type);
	}
	else if (rune_name == "DamageBuffRuneTier3")
	{
		mod_rune_type<false, DamageBuffRuneTier3, octopus::Attack>(e, add, type);
	}
	else if (rune_name == "AttackSpeedBuffRune")
	{
		mod_rune_type<false, AttackSpeedBuffRune, octopus::Attack>(e, add, type);
	}
	else if (rune_name == "AttackSpeedBuffRuneTier2")
	{
		mod_rune_type<false, AttackSpeedBuffRuneTier2, octopus::Attack>(e, add, type);
	}
	else if (rune_name == "AttackSpeedBuffRuneTier3")
	{
		mod_rune_type<false, AttackSpeedBuffRuneTier3, octopus::Attack>(e, add, type);
	}
	else if (rune_name == "TargetHasHighHpBonusDamageRune")
	{
		mod_rune_type<false, octopus::BuffAddComponent<TargetHasHighHpBonusDamageRune>>(e, add, type);
	}
	else if (rune_name == "TargetHasHighHpBonusDamageRuneTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<TargetHasHighHpBonusDamageRuneTier2>>(e, add, type);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnTargetOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnTargetOnAttack>>(e, add, type);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnTargetOnAttackTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnTargetOnAttackTier2>>(e, add, type);
	}
	else if (rune_name == "BonusDamageConsumeRuneOnTargetOnAttackTier3")
	{
		mod_rune_type<false, octopus::BuffAddComponent<BonusDamageConsumeRuneOnTargetOnAttackTier3>>(e, add, type);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnTargetOnAttack")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnTargetOnAttack>>(e, add, type);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnTargetOnAttackTier2")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnTargetOnAttackTier2>>(e, add, type);
	}
	else if (rune_name == "LifestealRune")
	{
		mod_rune_type<false, octopus::BuffAddComponent<LifestealRune>>(e, add, type);
	}
	else if (rune_name == "LifestealRuneConsumeRuneOnTarget")
	{
		mod_rune_type<false, octopus::BuffAddComponent<LifestealRuneConsumeRuneOnTarget>>(e, add, type);
	}
	else if (rune_name == "AoeDamageOnDeath")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageOnDeath>>(e, add, type);
	}
	else if (rune_name == "AoeHealOnDeath")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealOnDeath>>(e, add, type);
	}
	else if (rune_name == "AoeDamageConsumeRuneOnDeath")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageConsumeRuneOnDeath>>(e, add, type);
	}
	else if (rune_name == "AoeHealConsumeRuneOnDeath")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealConsumeRuneOnDeath>>(e, add, type);
	}
	else if (rune_name == "AddAoeRuneLoadToEnemiesOnDeath")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AddAoeRuneLoadToEnemiesOnDeath>>(e, add, type);
	}
	else if (rune_name == "AddAoeRuneLoadToAlliesOnDeath")
	{
		mod_rune_type<false, octopus::BuffAddComponent<AddAoeRuneLoadToAlliesOnDeath>>(e, add, type);
	}
}
