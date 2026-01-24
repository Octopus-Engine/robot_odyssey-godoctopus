#pragma once
#include "octopus/components/basic/attack/Attack.hh"

template<int32_t base_quantity>
struct DamageBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Attack &atk) const
	{
		atk.cst.damage += quantity;
	}

	void revert(flecs::entity, octopus::Attack &atk) const
	{
		atk.cst.damage -= quantity;
	}
};

template<int32_t base_quantity, int32_t level_gain>
struct LeveledDamageBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Fixed const &level, octopus::Attack &atk) const
	{
		atk.cst.damage += quantity + (level * level_gain);
	}

	void revert(flecs::entity, octopus::Fixed const &level, octopus::Attack &atk) const
	{
		atk.cst.damage -= quantity + (level * level_gain);
	}
};
