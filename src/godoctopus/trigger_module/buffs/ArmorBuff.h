#pragma once
#include "octopus/components/basic/armor/Armor.hh"

template<int32_t base_quantity>
struct ArmorBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Armor &armor) const
	{
		armor.qty += octopus::Fixed(quantity)/10;
	}

	void revert(flecs::entity, octopus::Armor &armor) const
	{
		armor.qty -= octopus::Fixed(quantity)/10;
	}
};

template<int32_t base_quantity, int32_t level_gain>
struct LeveledArmorBuff
{
	int32_t quantity = base_quantity;

	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void apply(flecs::entity, octopus::Fixed const &level, octopus::Armor &armor, Components&... components) const
	{
		armor.qty += octopus::Fixed(quantity + (level * level_gain))/10;
	}

	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void revert(flecs::entity, octopus::Fixed const &level, octopus::Armor &armor, Components&... components) const
	{
		armor.qty -= octopus::Fixed(quantity + (level * level_gain))/10;
	}
};
