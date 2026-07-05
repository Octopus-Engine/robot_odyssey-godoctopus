#include "TemporaryBuffTriggerDeclaration.h"

#include "TriggerTypes.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"
#include "octopus/components/advanced/buff/BuffSystem.hh"

#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/conditions/RuneCondition.h"
#include "godoctopus/trigger_module/events/TemporaryBuffEvent.h"
#include "octopus_types.h"

template<typename BuffType, typename ComponentType>
void declare_temporary_buff_systems(flecs::world &ecs, custom_step_manager& manager) {
	ecs.template component<BuffType>()
		.member("bonus", &BuffType::bonus)
	;

	octopus::declare_buff_system<BuffType>(ecs, manager);
	octopus::declare_stats_buff_systems<BuffType, ComponentType>(
		ecs,
		[](BuffType const& buff, ComponentType &atk) {
			buff.apply(atk);
		},
		[](BuffType const& buff, ComponentType &atk) {
			buff.revert(atk);
		}
	);
}

template<typename RuneType>
void declare_rune_temporary_buff(flecs::world &ecs, custom_step_manager& manager) {
	ecs.template component<RuneType>()
		.member("level", &RuneType::level)
		.member("base", &RuneType::base)
		.member("upgrade", &RuneType::upgrade)
		.member("duration_ticks", &RuneType::duration_ticks)
	;

	ecs.component<octopus::BuffAddComponent<RuneType>>()
		.member("placeholder", &octopus::BuffAddComponent<RuneType>::placeholder)
	;

	declare_player_buff_systems_all_units<typename octopus::BuffAddComponent<RuneType> >(ecs, false);

	declare_temporary_buff_systems<typename RuneType::BuffType, typename RuneType::BuffType::ComponentType>(ecs, manager);
}

void declare_temporary_buff_triggers(flecs::world &ecs, custom_step_manager& manager, octopus::PositionContext const &ctx)
{

	// declare temporary buff runes
	declare_rune_temporary_buff<ApplyArmorBuffOnRuneLoad>(ecs, manager);
	declare_rune_temporary_buff<ApplyArmorBuffAreaOnRuneLoad>(ecs, manager);
	declare_rune_temporary_buff<ApplyArmorDebuffAreaOnRuneLoad>(ecs, manager);

	declare_rune_temporary_buff<ApplyAttackSpeedBuffOnRuneLoad>(ecs, manager);
	declare_rune_temporary_buff<ApplyAttackSpeedBuffAreaOnRuneLoad>(ecs, manager);
	declare_rune_temporary_buff<ApplyAttackSpeedDebuffAreaOnRuneLoad>(ecs, manager);

	declare_rune_temporary_buff<ApplyDamageBuffOnRuneLoad>(ecs, manager);
	declare_rune_temporary_buff<ApplyDamageBuffAreaOnRuneLoad>(ecs, manager);
	declare_rune_temporary_buff<ApplyDamageDebuffAreaOnRuneLoad>(ecs, manager);

	ecs.component<ApplyArmorBuffAreaOnRuneLoad>().member("range", &ApplyArmorBuffAreaOnRuneLoad::range);
	ecs.component<ApplyArmorDebuffAreaOnRuneLoad>().member("range", &ApplyArmorDebuffAreaOnRuneLoad::range);

	ecs.component<ApplyAttackSpeedBuffAreaOnRuneLoad>().member("range", &ApplyAttackSpeedBuffAreaOnRuneLoad::range);
	ecs.component<ApplyAttackSpeedDebuffAreaOnRuneLoad>().member("range", &ApplyAttackSpeedDebuffAreaOnRuneLoad::range);

	ecs.component<ApplyDamageBuffAreaOnRuneLoad>().member("range", &ApplyDamageBuffAreaOnRuneLoad::range);
	ecs.component<ApplyDamageDebuffAreaOnRuneLoad>().member("range", &ApplyDamageDebuffAreaOnRuneLoad::range);

	// Self-buffs: consume 3 runes when triggered
	declare_trigger_system<ApplyDamageBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<ApplyDamageBuffOnRuneLoad>>(ecs);
	declare_trigger_system<ApplyAttackSpeedBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<ApplyAttackSpeedBuffOnRuneLoad>>(ecs);
	declare_trigger_system<ApplyArmorBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<ApplyArmorBuffOnRuneLoad>>(ecs);

	// Area-buffs: allies around unit
	declare_area_trigger_system<ApplyArmorBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryBuffAreaEvent<ApplyArmorBuffAreaOnRuneLoad>>(ecs, ctx);
	declare_area_trigger_system<ApplyDamageBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryBuffAreaEvent<ApplyDamageBuffAreaOnRuneLoad>>(ecs, ctx);
	declare_area_trigger_system<ApplyAttackSpeedBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryBuffAreaEvent<ApplyAttackSpeedBuffAreaOnRuneLoad>>(ecs, ctx);

	// Area-debuffs: enemies around unit
	declare_area_trigger_system<ApplyArmorDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryDebuffAreaEvent<ApplyArmorDebuffAreaOnRuneLoad>>(ecs, ctx);
	declare_area_trigger_system<ApplyDamageDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryDebuffAreaEvent<ApplyDamageDebuffAreaOnRuneLoad>>(ecs, ctx);
	declare_area_trigger_system<ApplyAttackSpeedDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryDebuffAreaEvent<ApplyAttackSpeedDebuffAreaOnRuneLoad>>(ecs, ctx);
}

