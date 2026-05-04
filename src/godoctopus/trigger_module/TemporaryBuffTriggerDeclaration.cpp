#include "TemporaryBuffTriggerDeclaration.h"

#include "TriggerTypes.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"
#include "octopus/components/advanced/buff/BuffSystem.hh"

#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/conditions/RuneCondition.h"
#include "godoctopus/trigger_module/events/TemporaryBuffEvent.h"
#include "godoctopus/trigger_module/buffs/TemporaryStatsBuff.h"
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

void declare_temporary_buff_triggers(flecs::world &ecs, custom_step_manager& manager, octopus::PositionContext const &ctx)
{
	// declare temporary buff trigger systems
	using TemporaryArmorBuff_10_1_15 = TemporaryArmorBuff<10, 1, 15*TICK_RATE>;
	using TemporaryDamageBuff_20_2_15 = TemporaryDamageBuff<20, 2, 15*TICK_RATE>;
	using TemporaryAttackSpeedBuff_10_1_15 = TemporaryAttackSpeedBuff<10, 1, 15*TICK_RATE>;
	using TemporaryArmorDebuff_10_1_15 = TemporaryArmorBuff<-10, -1, 15*TICK_RATE>;
	using TemporaryDamageDebuff_20_2_15 = TemporaryDamageBuff<-20, -2, 15*TICK_RATE>;
	using TemporaryAttackSpeedDebuff_30_3_15 = TemporaryAttackSpeedBuff<-30, -3, 15*TICK_RATE>;

	// Armor buff
	declare_temporary_buff_systems<TemporaryArmorBuff_10_1_15, octopus::Armor>(ecs, manager);
	declare_temporary_buff_systems<TemporaryArmorDebuff_10_1_15, octopus::Armor>(ecs, manager);

	// Damage buff
	declare_temporary_buff_systems<TemporaryDamageBuff_20_2_15, octopus::Attack>(ecs, manager);
	declare_temporary_buff_systems<TemporaryDamageDebuff_20_2_15, octopus::Attack>(ecs, manager);

	// Attack speed buff
	declare_temporary_buff_systems<TemporaryAttackSpeedBuff_10_1_15, octopus::Attack>(ecs, manager);
	declare_temporary_buff_systems<TemporaryAttackSpeedDebuff_30_3_15, octopus::Attack>(ecs, manager);

	// Self-buffs: consume 3 runes when triggered
	declare_trigger_system<ApplyArmorBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<TemporaryArmorBuff_10_1_15>>(ecs);
	declare_trigger_system<ApplyDamageBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<TemporaryDamageBuff_20_2_15>>(ecs);
	declare_trigger_system<ApplyAttackSpeedBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<TemporaryAttackSpeedBuff_10_1_15>>(ecs);

	// Area-buffs: allies around unit
	declare_area_trigger_system<ApplyArmorBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryBuffAreaEvent<TemporaryArmorBuff_10_1_15, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyDamageBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryBuffAreaEvent<TemporaryDamageBuff_20_2_15, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyAttackSpeedBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryBuffAreaEvent<TemporaryAttackSpeedBuff_10_1_15, 5>>(ecs, ctx);

	// Area-debuffs: enemies around unit
	declare_area_trigger_system<ApplyArmorDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryDebuffAreaEvent<TemporaryArmorDebuff_10_1_15, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyDamageDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryDebuffAreaEvent<TemporaryDamageDebuff_20_2_15, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyAttackSpeedDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<5, DefaultRune>, ApplyTemporaryDebuffAreaEvent<TemporaryAttackSpeedDebuff_30_3_15, 5>>(ecs, ctx);
}

