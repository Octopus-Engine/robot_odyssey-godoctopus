#include "TriggerDeclaration.h"

#include "core/variant/variant.h"
#include "core/string/print_string.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"
#include "octopus/systems/phases/Phases.hh"

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

template<typename BuffType, typename... ComponentTypes>
struct UpdateableBuffSystemDeclarer {
	flecs::world &ecs;

	template<typename BotType>
	void operator()() const {
		flecs::query query_units = ecs.query_builder<octopus::PlayerAppartenance const, SpecialUpdate<BuffType>, ComponentTypes...>()
			.template with<BotType>()
			.build();

		ecs.component<SpecialUpdate<BuffType>>()
			.member("old_special", &SpecialUpdate<BuffType>::old_special)
			.member("init", &SpecialUpdate<BuffType>::init);

		// Lazy set up of component
		ecs.system<octopus::PlayerAppartenance const, ComponentTypes...>()
			.kind(ecs.entity(ValidatePhase))
			.template without<SpecialUpdate<BuffType>>()
			.template write<SpecialUpdate<BuffType>>()
			.each([](flecs::entity e, octopus::PlayerAppartenance const &player_appartenance, ComponentTypes&... component) {
				e.add<SpecialUpdate<BuffType>>();
			});

		ecs.system<octopus::PlayerInfo const, octopus::PlayerBuff<BotType, BuffType, ComponentTypes...> >()
			.kind(ecs.entity(ValidatePhase))
			.each([query_units] (octopus::PlayerInfo const &player, octopus::PlayerBuff<BotType, BuffType, ComponentTypes...> &player_buff) {
				query_units.each([&](flecs::entity e, octopus::PlayerAppartenance const &player_appartenance, SpecialUpdate<BuffType> & spec_up, ComponentTypes&... component)
				{
					if(player_appartenance.idx != player.idx) { return; }
					player_buff.buff.update_value(e, spec_up, component ...);
				});
			});
	}
};

template<typename BuffType, typename... ComponentType>
void declare_updatable_buff(flecs::world &ecs)
{
	// component declaration
	ecs.component<BuffType>()
		.member("quantity", &BuffType::quantity);

	declare_player_buff_systems_all_units<BuffType, ComponentType...>(ecs, true);

	// declare validate system that will update buff value during validate phase
	for_each_bot_type(UpdateableBuffSystemDeclarer<BuffType, ComponentType...>{ecs});
}

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx)
{
	ecs.component<trigger_module::Death>();
	ecs.component<trigger_module::Attack>();
	ecs.component<trigger_module::DamageDealt>();
	ecs.component<trigger_module::DamageReceived>();
	ecs.component<trigger_module::Death>();
	ecs.component<trigger_module::RuneConsumed<DefaultRune>>();
	ecs.component<trigger_module::RuneLoaded<DefaultRune>>();

	declare_trigger_life_system(ecs);
	declare_trigger_attack_system(ecs);

	declare_trigger_system<AddRuneLoadOnAttack, trigger_module::DamageDealt, AlwaysCondition, RuneEvent<DefaultRune, 1>>(ecs);
	declare_attack_trigger_system<AddRuneLoadOnTargetOnAttack, trigger_module::DamageDealt, AlwaysCondition, TargetEvent<RuneEvent<DefaultRune, 1>>>(ecs);

	declare_trigger_system<HealAndConsumeRuneLoadOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, HitpointEvent<5,1>>(ecs);
	declare_trigger_system<HealAndConsumeRuneLoadOnAttack, trigger_module::Attack, RuneCondition<1, DefaultRune>, HitpointEvent<5,1>>(ecs);
	declare_trigger_system<HealAndConsumeRuneLoadOnAttackTier2, trigger_module::Attack, RuneCondition<2, DefaultRune>, HitpointEvent<10,2>>(ecs);

	declare_attack_trigger_system<BonusDamageConsumeRuneOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, FactorDamageEvent<20, 2>>(ecs);
	declare_attack_trigger_system<BonusDamageRune, trigger_module::DamageDealt, AlwaysCondition, FactorDamageEventSpecialScaled<10, 1>>(ecs);
	declare_attack_trigger_system<HasLowHpDoubleDamageRune, trigger_module::DamageDealt, HitPointUnderPercentCondition<25>, FactorDamageEvent<100, 25>>(ecs);
	declare_attack_trigger_system<HasHighHpBonusDamageRune, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<50, 25>>(ecs);

	declare_attack_trigger_system<BonusDamageSelfDamage, trigger_module::DamageDealt, HitPointCostCondition<2>, FactorDamageEvent<150, 50>>(ecs);

	declare_area_trigger_system<AoeDamageOnHit, trigger_module::DamageReceived, AlwaysCondition, DamageAreaEvent<1, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageOnHitLevel, trigger_module::DamageReceived, AlwaysCondition, DamageAreaEvent<1, 5, 1>>(ecs, ctx);

	declare_area_trigger_system<AoeDamageConsumeRuneOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, DamageAreaEventSpecialScaled<2, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageConsumeRuneOnHitTier2, trigger_module::DamageReceived, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	declare_area_trigger_system<AoeHealOnHit, trigger_module::DamageReceived, AlwaysCondition, HealAreaEvent<2, 5, 1>>(ecs, ctx);

	declare_area_trigger_system<AoeHealConsumeRuneOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, HealAreaEventSpecialScaled<4, 5, 2>>(ecs, ctx);
	declare_area_trigger_system<AoeHealConsumeRuneOnHitTier2, trigger_module::DamageReceived, RuneCondition<2, DefaultRune>, HealAreaEvent<8, 5, 4>>(ecs, ctx);

	declare_area_trigger_system<AoeHealOnAttack, trigger_module::DamageDealt, AlwaysCondition, HealAreaEvent<8, 2, 4>>(ecs, ctx);

	declare_attack_area_trigger_system<AoeDamageSpecial, trigger_module::DamageDealt, AlwaysCondition, DamageAreaEventSpecialScaled<10, 5, 1>>(ecs, ctx);
	declare_attack_area_trigger_system<AoeDamageConsumeRuneOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	declare_attack_area_trigger_system<AoeDamageConsumeRuneOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	declare_area_trigger_system<AoeRuneToEnnemiesOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, false>>(ecs, ctx);

	declare_area_trigger_system<AoeRuneToAlliesOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, true>>(ecs, ctx);

	declare_trigger_system<AddRuneLoadOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneEvent<DefaultRune, 1>>(ecs);

	declare_attack_trigger_target_condition_system<TargetHasHighHpBonusDamageRune, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<50, 10>>(ecs);
	declare_attack_trigger_target_condition_system<TargetHasHighHpBonusDamageRuneTier2, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<100, 20>>(ecs);
	declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, FactorDamageEvent<30, 3>>(ecs);
	declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, FactorDamageEvent<200, 50>>(ecs);
	declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttackTier3, trigger_module::DamageDealt, RuneCondition<3, DefaultRune>, FactorDamageEvent<300, 100>>(ecs);

	declare_attack_area_trigger_target_condition_system<AoeDamageConsumeRuneOnTargetOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	declare_attack_area_trigger_target_condition_system<AoeDamageConsumeRuneOnTargetOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	declare_attack_trigger_system<LifestealRuneCore, trigger_module::DamageDealt, AlwaysCondition, LifestealEventSpecialScaled<5, 1> >(ecs);
	declare_attack_trigger_system<LifestealRuneSpecial, trigger_module::DamageDealt, AlwaysCondition, LifestealEventSpecialScaled<10, 2> >(ecs);
	declare_attack_trigger_target_condition_system<LifestealRuneConsumeRuneOnTarget, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, LifestealEvent<20, 4> >(ecs);

	declare_area_trigger_system<AoeDamageOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEventHitpointBasedSpecialScaled<19, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeHealOnDeath, trigger_module::Death, AlwaysCondition, HealAreaEvent<10, 5, 5>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageConsumeRuneOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEvent<8, 5, 8>>(ecs, ctx);
	declare_area_trigger_system<AoeHealConsumeRuneOnDeath, trigger_module::Death, RuneCondition<1, DefaultRune>, HealAreaEvent<15, 5, 10>>(ecs, ctx);
	declare_area_trigger_system<AddAoeRuneLoadToEnemiesOnDeath, trigger_module::Death, RuneCondition<1, DefaultRune>, RuneAreaEvent<DefaultRune, 1, 5, false>>(ecs, ctx);
	declare_area_trigger_system<AddAoeRuneLoadToAlliesOnDeath, trigger_module::Death, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, true>>(ecs, ctx);

	// declare all trigger buff systems
	declare_trigger_buff<AddRuneLoadOnAttack>(ecs);
	declare_trigger_buff<AddRuneLoadOnTargetOnAttack>(ecs);
	declare_trigger_buff<HealAndConsumeRuneLoadOnHit>(ecs);
	declare_trigger_buff<HealAndConsumeRuneLoadOnAttack>(ecs);
	declare_trigger_buff<HealAndConsumeRuneLoadOnAttackTier2>(ecs);
	declare_trigger_buff<BonusDamageConsumeRuneOnAttack>(ecs);
	declare_trigger_buff<BonusDamageRune>(ecs);
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
	declare_trigger_buff<AoeDamageSpecial>(ecs);
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
	// declare_classic_buff<HitPointBuffRune, octopus::HitPoint, octopus::HitPointMax>(ecs);
	// declare_classic_buff<HitPointBuffRuneTier2, octopus::HitPoint, octopus::HitPointMax>(ecs);
	// declare_classic_buff<HitPointBuffRuneTier3, octopus::HitPoint, octopus::HitPointMax>(ecs);
	// declare_classic_buff<DamageBuffRune, octopus::Attack>(ecs);
	// declare_classic_buff<DamageBuffRuneTier2, octopus::Attack>(ecs);
	// declare_classic_buff<DamageBuffRuneTier3, octopus::Attack>(ecs);
	// declare_classic_buff<AttackSpeedBuffRune, octopus::Attack>(ecs);
	// declare_classic_buff<AttackSpeedBuffRuneTier2, octopus::Attack>(ecs);
	// declare_classic_buff<AttackSpeedBuffRuneTier3, octopus::Attack>(ecs);

	// declare scaling buffs
	// regulars
	declare_classic_buff<HitPointBuffRuneRegular, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_classic_buff<ArmorBuffRuneRegular, octopus::Armor>(ecs);
	declare_classic_buff<DamageBuffRuneRegular, octopus::Attack>(ecs);
	declare_classic_buff<ReloadBuffRuneRegular, octopus::Attack>(ecs);
	declare_classic_buff<SpecialBuffRuneRegular, Special>(ecs);
	declare_classic_buff<AffinityBuffRuneRegular, Special>(ecs);
	// cores
	declare_updatable_buff<HitPointBuffRuneCore, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_updatable_buff<ArmorBuffRuneCore, octopus::Armor>(ecs);
	declare_updatable_buff<DamageBuffRuneCore, octopus::Attack>(ecs);
	declare_updatable_buff<ReloadBuffRuneCore, octopus::Attack>(ecs);
	// specials
	declare_updatable_buff<HitPointBuffRuneSpecial, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_updatable_buff<ArmorBuffRuneSpecial, octopus::Armor>(ecs);
	declare_updatable_buff<DamageBuffRuneSpecial, octopus::Attack>(ecs);
	declare_updatable_buff<ReloadBuffRuneSpecial, octopus::Attack>(ecs);
}

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
	else if (rune_name == "AoeDamageOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeDamageOnDeath>>(e, add, type, level);
	}
	else if (rune_name == "AoeHealOnDeath") {
		mod_rune_type<false, octopus::BuffAddComponent<AoeHealOnDeath>>(e, add, type, level);
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
	else {
		print_line("mod_rune_based_on_names: Unknown rune name ", rune_name.c_str());
	}
}
