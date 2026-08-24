#pragma once

#include "octopus/commands/basic/move/AttackCommandSystem.hh"
#include "octopus/components/basic/hitpoint/Destroyable.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "godoctopus/components/rune_load/RuneLoad.h"

namespace trigger_module
{
	//////////////////////
	// events types     //
	//////////////////////

	/// @brief when a unit attacks
	struct Attack
	{
		typedef octopus::AttackTrigger component;
		octopus::Fixed damage;
	};
	/// @brief when damage is dealt to a unit (event push to the damage dealer)
	struct DamageDealt
	{
		typedef DamageDealt component;
		octopus::Fixed amount;
	};
	/// @brief when damage is received by a unit (event push to the damage receiver)
	struct DamageReceived
	{
		typedef DamageReceived component;
		octopus::Fixed amount;
	};
	/// @brief when unit dies
	struct Death
	{
		typedef octopus::Destroyable component;
	};
	/// @brief when a rune load is consumed
	template<typename type>
	struct RuneConsumed
	{
		typedef RuneLoad<type> component;
	};
	/// @brief when a rune load is added
	template<typename type>
	struct RuneLoaded
	{
		typedef RuneLoad<type> component;
	};

	inline void emit_damage_triggers(flecs::entity const &attacker, flecs::entity const &target, octopus::Fixed const &damage) {
		flecs::world ecs = attacker.world();
		auto *damage_delt = attacker.try_get_mut<DamageDealt>();
		if (damage_delt) {
			damage_delt->amount = damage;
			// emit damage dealt event to attacker
			ecs.event<DamageDealt>()
				.id<DamageDealt>()
				.entity(attacker)
				.emit();
		}
		auto *damage_recv = target.try_get_mut<DamageReceived>();
		if (damage_recv) {
			damage_recv->amount = damage;
			// emit damage received event to target
			ecs.event<DamageReceived>()
				.id<DamageReceived>()
				.entity(target)
				.emit();
		}
	}
}
