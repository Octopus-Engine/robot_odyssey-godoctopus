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

	// Armor buff
	declare_temporary_buff_systems<TemporaryArmorBuff<10, 15*TICK_RATE>, octopus::Armor>(ecs, manager);
	declare_temporary_buff_systems<TemporaryArmorBuff<-10, 15*TICK_RATE>, octopus::Armor>(ecs, manager);

	// Damage buff
	declare_temporary_buff_systems<TemporaryDamageBuff<20, 15*TICK_RATE>, octopus::Attack>(ecs, manager);
	declare_temporary_buff_systems<TemporaryDamageBuff<-20, 15*TICK_RATE>, octopus::Attack>(ecs, manager);

	// Attack speed buff
	declare_temporary_buff_systems<TemporaryAttackSpeedBuff<30, 15*TICK_RATE>, octopus::Attack>(ecs, manager);
	declare_temporary_buff_systems<TemporaryAttackSpeedBuff<-30, 15*TICK_RATE>, octopus::Attack>(ecs, manager);

	// Self-buffs: consume 3 runes when triggered
	declare_trigger_system<ApplyArmorBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<TemporaryArmorBuff<10, 15*TICK_RATE>>>(ecs);
	declare_trigger_system<ApplyDamageBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<TemporaryDamageBuff<20, 15*TICK_RATE>>>(ecs);
	declare_trigger_system<ApplyAttackSpeedBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffEvent<TemporaryAttackSpeedBuff<30, 15*TICK_RATE>>>(ecs);

	// Area-buffs: allies around unit
	declare_area_trigger_system<ApplyArmorBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffAreaEvent<TemporaryArmorBuff<10, 15*TICK_RATE>, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyDamageBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffAreaEvent<TemporaryDamageBuff<20, 15*TICK_RATE>, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyAttackSpeedBuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryBuffAreaEvent<TemporaryAttackSpeedBuff<30, 15*TICK_RATE>, 5>>(ecs, ctx);

	// Area-debuffs: enemies around unit
	declare_area_trigger_system<ApplyArmorDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryDebuffAreaEvent<TemporaryArmorBuff<-10, 15*TICK_RATE>, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyDamageDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryDebuffAreaEvent<TemporaryDamageBuff<-20, 15*TICK_RATE>, 5>>(ecs, ctx);
	declare_area_trigger_system<ApplyAttackSpeedDebuffAreaOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, RuneCondition<3, DefaultRune>, ApplyTemporaryDebuffAreaEvent<TemporaryAttackSpeedBuff<-30, 15*TICK_RATE>, 5>>(ecs, ctx);
}
