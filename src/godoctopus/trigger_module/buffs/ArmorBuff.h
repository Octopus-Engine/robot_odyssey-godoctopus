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

	void apply(flecs::entity, octopus::Fixed const &level, octopus::Armor &armor) const
	{
		armor.qty += octopus::Fixed(quantity + (level * level_gain))/10;
	}

	void revert(flecs::entity, octopus::Fixed const &level, octopus::Armor &armor) const
	{
		armor.qty -= octopus::Fixed(quantity + (level * level_gain))/10;
	}
};
