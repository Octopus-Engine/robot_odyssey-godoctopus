#pragma once
#include "octopus/components/basic/attack/Attack.hh"

struct LeveledDamageBuff
{
	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void apply(flecs::entity, octopus::Fixed const &qty, octopus::Attack &atk, Components&... components) const
	{
		atk.cst.damage += qty;
	}

	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void revert(flecs::entity, octopus::Fixed const &qty, octopus::Attack &atk, Components&... components) const
	{
		atk.cst.damage -= qty;
	}
};
