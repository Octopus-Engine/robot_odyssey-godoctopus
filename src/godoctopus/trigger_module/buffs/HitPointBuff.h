#pragma once
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

template<int32_t base_quantity>
struct HitPointBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty += quantity;
		hp_max.qty += quantity;
	}

	void revert(flecs::entity, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty -= quantity;
		hp_max.qty -= quantity;
	}
};

template<int32_t base_quantity, int32_t level_gain>
struct LeveledHitPointBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Fixed const &level, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty += quantity + (level * level_gain);
		hp_max.qty += quantity + (level * level_gain);;
	}

	void revert(flecs::entity, octopus::Fixed const &level, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty -= quantity + (level * level_gain);
		hp_max.qty -= quantity + (level * level_gain);;
	}
};
