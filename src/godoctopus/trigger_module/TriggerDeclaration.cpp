#include "TriggerDeclaration.h"

#include "core/variant/variant.h"
#include "core/string/print_string.h"

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
#include "godoctopus/trigger_module/SpawnUnitRune.h"
#include "godoctopus/trigger_module/UndyingRune.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"

template<typename BuffType, typename... ComponentType>
void declare_classic_buff(flecs::world &ecs)
{
	// component declaration
	ecs.component<BuffType>()
		.member("qty", &BuffType::qty);

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
			.member("active", &SpecialUpdate<BuffType>::active);

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
		.member("base", &BuffType::base)
		.member("upgrade", &BuffType::upgrade);

	declare_player_buff_systems_all_units<BuffType, ComponentType...>(ecs, true);

	// declare validate system that will update buff value during validate phase
	for_each_bot_type(UpdateableBuffSystemDeclarer<BuffType, ComponentType...>{ecs});
}

template<typename BuffType, typename... ComponentType>
void declare_conditional_updatable_buff(flecs::world &ecs)
{
	ecs.component<BuffType>()
		.member("percent", &BuffType::percent)
	;
	declare_updatable_buff<BuffType, ComponentType...>(ecs);
}

void declare_triggers(flecs::world &ecs, octopus::PositionContext const &ctx, custom_step_manager &manager, godot::SmartMMeshLibrary *library)
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

	declare_attack_trigger_system<LifestealRuneSpecial, trigger_module::DamageDealt, AlwaysCondition, LifestealEventSpecialScaled>(ecs);
	declare_attack_area_trigger_system<AoeDamageSpecial, trigger_module::DamageDealt, AlwaysCondition, DamageAreaEventSpecialScaled>(ecs, ctx);

	declare_area_trigger_system<AoeDamageBasedOnHitpointOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEventHitpointBasedSpecialScaled>(ecs, ctx);
	declare_area_trigger_system<AoeDamageBasedOnDamageOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEventDamageBasedSpecialScaled>(ecs, ctx);
	declare_area_trigger_system<AoeHealBasedOnHitpointOnDeath, trigger_module::Death, AlwaysCondition, HealAreaEventHitpointBasedSpecialScaled>(ecs, ctx);
	declare_area_trigger_system<AoeHealBasedOnDamageOnDeath, trigger_module::Death, AlwaysCondition, HealAreaEventDamageBasedSpecialScaled>(ecs, ctx);
	declare_area_trigger_system<AoeDamageConsumeRuneOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, DamageAreaEventSpecialScaled>(ecs, ctx);

	// declare all trigger buff systems
	declare_trigger_buff<AddRuneLoadOnAttack>(ecs);
	declare_trigger_buff<AddRuneLoadOnTargetOnAttack>(ecs);
	declare_trigger_buff<AddRuneLoadOnHit>(ecs);
	declare_trigger_buff<LifestealRuneSpecial>(ecs);
	declare_trigger_buff<AoeDamageSpecial>(ecs);
	declare_trigger_buff<AoeDamageBasedOnHitpointOnDeath>(ecs);
	declare_trigger_buff<AoeDamageBasedOnDamageOnDeath>(ecs);
	declare_trigger_buff<AoeHealBasedOnHitpointOnDeath>(ecs);
	declare_trigger_buff<AoeHealBasedOnDamageOnDeath>(ecs);
	declare_trigger_buff<AoeDamageConsumeRuneOnHit>(ecs);

	// declare scaling buffs
	// regulars
	declare_classic_buff<AffinityBuffRuneRegular, godoctopus::BaseStats>(ecs);
	declare_classic_buff<ArmorBuffRuneRegular, godoctopus::BaseStats>(ecs);
	declare_classic_buff<DamageBuffRuneRegular, godoctopus::BaseStats>(ecs);
	declare_classic_buff<HitPointBuffRuneRegular, godoctopus::BaseStats>(ecs);
	declare_classic_buff<ReloadBuffRuneRegular, godoctopus::BaseStats>(ecs);
	// specials
	declare_updatable_buff<HitPointBuffRuneSpecial, godoctopus::BaseStats>(ecs);
	declare_updatable_buff<ArmorBuffRuneSpecial, godoctopus::BaseStats>(ecs);
	declare_updatable_buff<DamageBuffRuneSpecial, godoctopus::BaseStats>(ecs);
	declare_updatable_buff<ReloadBuffRuneSpecial, godoctopus::BaseStats>(ecs);

	// Conditional armor buff
	declare_conditional_updatable_buff<ConditionalArmorBuffLowLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalArmorBuffLowLifeRuneTier2, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalDamageBuffLowLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalDamageBuffLowLifeRuneTier2, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalReloadBuffLowLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalReloadBuffLowLifeRuneTier2, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);

	declare_conditional_updatable_buff<ConditionalArmorBuffHighLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalArmorBuffHighLifeRuneTier2, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalDamageBuffHighLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalDamageBuffHighLifeRuneTier2, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalReloadBuffHighLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);
	declare_conditional_updatable_buff<ConditionalReloadBuffHighLifeRuneTier2, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(ecs);

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

	declare_temporary_buff_triggers(ecs, manager, ctx);

	declare_undying_rune_triggers(ecs, manager, library);

	declare_spawn_unit_triggers(ecs, ctx);
}
