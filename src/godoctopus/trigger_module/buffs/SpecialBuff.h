#pragma once

#include "godoctopus/components/special/Special.h"

template<int32_t base_quantity>
struct SpecialBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, Special &spec) const
	{
		spec.value += quantity;
	}

	void revert(flecs::entity, Special &spec) const
	{
		spec.value -= quantity;
	}
};

template<int32_t base_quantity, int32_t level_gain>
struct LeveledSpecialBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Fixed const &level, Special &spec) const
	{
		spec.value += quantity + (level * level_gain);
	}

	void revert(flecs::entity, octopus::Fixed const &level, Special &spec) const
	{
		spec.value -= quantity + (level * level_gain);
	}
};

template<int32_t base_quantity>
struct AffinityBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, Special &spec) const
	{
		spec.affinity += octopus::Fixed(quantity)/10;
	}

	void revert(flecs::entity, Special &spec) const
	{
		spec.affinity -= octopus::Fixed(quantity)/10;
	}
};

template<int32_t base_quantity, int32_t level_gain>
struct LeveledAffinityBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Fixed const &level, Special &spec) const
	{
		spec.affinity += octopus::Fixed(quantity + (level * level_gain))/10;
	}

	void revert(flecs::entity, octopus::Fixed const &level, Special &spec) const
	{
		spec.affinity -= octopus::Fixed(quantity + (level * level_gain))/10;
	}
};
