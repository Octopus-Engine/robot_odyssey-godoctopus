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
#include "godoctopus/trigger_module/AoePulseRune.h"

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

	declare_aoe_pulse_triggers(ecs, ctx);

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
