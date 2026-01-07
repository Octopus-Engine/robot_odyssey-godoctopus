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
	};
	/// @brief when damage is dealt to a unit (event push to the damage dealer)
	struct DamageDealt
	{
		typedef octopus::AttackTrigger component;
	};
	/// @brief when damage is received by a unit (event push to the damage receiver)
	struct DamageReceived
	{
		typedef octopus::HitPoint component;
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
}
