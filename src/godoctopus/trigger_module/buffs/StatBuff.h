#pragma once

#include "godoctopus/components/stats/StatsUpdateSystems.h"

template<godoctopus::StatsType type>
struct LeveledStatBuff
{
	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void apply(flecs::entity, octopus::Fixed const &qty, godoctopus::BaseStats &base, Components&... components) const
	{
		base.stats.values[type] += qty;
	}

	// Template parameters are used to allow passing extra components that can be
	// used for the condition of the buff, but they are not used
	template<typename... Components>
	void revert(flecs::entity, octopus::Fixed const &qty, godoctopus::BaseStats &base, Components&... components) const
	{
		base.stats.values[type] -= qty;
	}
};
