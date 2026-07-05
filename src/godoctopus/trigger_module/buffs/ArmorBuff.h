#pragma once
#include "octopus/components/basic/armor/Armor.hh"

struct LeveledArmorBuff
{
	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void apply(flecs::entity, octopus::Fixed const &qty, octopus::Armor &armor, Components&... components) const
	{
		armor.qty += qty/10;
	}

	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void revert(flecs::entity, octopus::Fixed const &qty, octopus::Armor &armor, Components&... components) const
	{
		armor.qty -= qty/10;
	}
};
