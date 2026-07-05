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

struct LeveledSpecialBuff
{
	void apply(flecs::entity, octopus::Fixed const &qty, Special &spec) const
	{
		spec.value += qty;
	}

	void revert(flecs::entity, octopus::Fixed const &qty, Special &spec) const
	{
		spec.value -= qty;
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

struct LeveledAffinityBuff
{
	void apply(flecs::entity, octopus::Fixed const &qty, Special &spec) const
	{
		spec.affinity += qty/10;
	}

	void revert(flecs::entity, octopus::Fixed const &qty, Special &spec) const
	{
		spec.affinity -= qty/10;
	}
};
