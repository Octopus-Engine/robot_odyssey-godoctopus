#include "TriggerDeclaration.h"

#include "core/variant/variant.h"
#include "core/string/print_string.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"
#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/advanced/buff/BuffSystem.hh"
#include "octopus/components/step/BuffComponentStep.hh"

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
#include "godoctopus/trigger_module/TemporaryBuffTriggerDeclaration.h"

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

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx, custom_step_manager &manager)
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
	declare_trigger_system<AddRuneLoadOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneEvent<DefaultRune, 1>>(ecs);

	declare_attack_trigger_system<LifestealRuneSpecial, trigger_module::DamageDealt, AlwaysCondition, LifestealEventSpecialScaled<10, 2> >(ecs);

	declare_area_trigger_system<AoeDamageBasedOnHitpointOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEventHitpointBasedSpecialScaled<19, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeDamageBasedOnDamageOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEventDamageBasedSpecialScaled<19, 5, 1>>(ecs, ctx);
	declare_area_trigger_system<AoeHealBasedOnHitpointOnDeath, trigger_module::Death, AlwaysCondition, HealAreaEventHitpointBasedSpecialScaled<10, 5, 5>>(ecs, ctx);
	declare_area_trigger_system<AoeHealBasedOnDamageOnDeath, trigger_module::Death, AlwaysCondition, HealAreaEventDamageBasedSpecialScaled<10, 5, 5>>(ecs, ctx);

	// declare all trigger buff systems
	declare_trigger_buff<AddRuneLoadOnAttack>(ecs);
	declare_trigger_buff<AddRuneLoadOnTargetOnAttack>(ecs);
	declare_trigger_buff<AddRuneLoadOnHit>(ecs);
	declare_trigger_buff<LifestealRuneSpecial>(ecs);
	declare_trigger_buff<AoeDamageBasedOnHitpointOnDeath>(ecs);
	declare_trigger_buff<AoeDamageBasedOnDamageOnDeath>(ecs);
	declare_trigger_buff<AoeHealBasedOnHitpointOnDeath>(ecs);
	declare_trigger_buff<AoeHealBasedOnDamageOnDeath>(ecs);

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

	// declare periodic pulse rune buff systems
	declare_trigger_buff<AoePulseHealBasedOnHitpoint>(ecs);
	declare_trigger_buff<AoePulseHealBasedOnHitpointTier2>(ecs);
	declare_trigger_buff<AoePulseHealBasedOnDamage>(ecs);
	declare_trigger_buff<AoePulseHealBasedOnDamageTier2>(ecs);
	declare_trigger_buff<AoePulseDamageBasedOnHitpoint>(ecs);
	declare_trigger_buff<AoePulseDamageBasedOnHitpointTier2>(ecs);
	declare_trigger_buff<AoePulseDamageBasedOnDamage>(ecs);
	declare_trigger_buff<AoePulseDamageBasedOnDamageTier2>(ecs);

	// declare periodic area trigger systems for pulse runes
	// Tier 1: base stats (5% base, 3% per level)
	declare_periodic_area_trigger_system<AoePulseHealBasedOnHitpoint, AlwaysCondition, HealAreaEventPeriodicHitpointBased<5, 3, 3>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseHealBasedOnDamage, AlwaysCondition, HealAreaEventPeriodicDamageBased<5, 3, 3>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnHitpoint, AlwaysCondition, DamageAreaEventPeriodicHitpointBased<5, 3, 3>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnDamage, AlwaysCondition, DamageAreaEventPeriodicDamageBased<5, 3, 3>>(ecs, ctx);

	// Tier 2: enhanced stats (10% base, 5% per level)
	declare_periodic_area_trigger_system<AoePulseHealBasedOnHitpointTier2, AlwaysCondition, HealAreaEventPeriodicHitpointBased<10, 3, 5>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseHealBasedOnDamageTier2, AlwaysCondition, HealAreaEventPeriodicDamageBased<10, 3, 5>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnHitpointTier2, AlwaysCondition, DamageAreaEventPeriodicHitpointBased<10, 3, 5>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnDamageTier2, AlwaysCondition, DamageAreaEventPeriodicDamageBased<10, 3, 5>>(ecs, ctx);

	// declare temporary buff rune buff systems
	declare_trigger_buff<ApplyHealthBuffOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyArmorBuffOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyDamageBuffOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyAttackSpeedBuffOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyHealthBuffAreaOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyArmorBuffAreaOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyDamageBuffAreaOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyAttackSpeedBuffAreaOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyArmorDebuffAreaOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyDamageDebuffAreaOnRuneLoad>(ecs);
	declare_trigger_buff<ApplyAttackSpeedDebuffAreaOnRuneLoad>(ecs);

	declare_temporary_buff_triggers(ecs, manager, ctx);
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
	else {
		print_line("mod_rune_based_on_names: Unknown rune name ", rune_name.c_str());
	}
}
