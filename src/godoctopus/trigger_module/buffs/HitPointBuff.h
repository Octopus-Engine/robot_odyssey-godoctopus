#pragma once
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

struct LeveledHitPointBuff
{
	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void apply(flecs::entity, octopus::Fixed const &qty, octopus::HitPoint &hp, octopus::HitPointMax &hp_max, Components&... components) const
	{
		hp.qty += qty;
		hp_max.qty += qty;
	}

	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void revert(flecs::entity, octopus::Fixed const &qty, octopus::HitPoint &hp, octopus::HitPointMax &hp_max, Components&... components) const
	{
		hp.qty -= qty;
		hp_max.qty -= qty;
	}
};
