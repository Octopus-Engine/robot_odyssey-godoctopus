#pragma once

#include "flecs.h"
#include "octopus/world/position/PositionContext.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/attack/Attack.hh"

// Base temporary buff event - applies buff to single entity
template<typename BuffType>
struct ApplyTemporaryBuffEvent
{
	static void apply(flecs::entity source, int32_t level)
	{
		// Apply buff directly to the entity
		source.set<octopus::BuffComponent<BuffType>>({BuffType(), octopus::get_time_stamp(source.world()), BuffType::DURATION_TICKS, false});
	}
};

// Apply a temporary buff to all allies in an area
template<typename BuffType, int32_t range>
struct ApplyTemporaryBuffAreaEvent
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level)
	{
		using namespace octopus;

		auto func_l = [team](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team == team && ent.try_get<HitPoint>())
			{
				ent.set<octopus::BuffComponent<BuffType>>({BuffType(), octopus::get_time_stamp(ent.world()), BuffType::DURATION_TICKS, false});
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};

// Apply a temporary debuff to all enemies in an area
template<typename BuffType, int32_t range>
struct ApplyTemporaryDebuffAreaEvent
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level)
	{
		using namespace octopus;

		auto func_l = [team](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team != team && ent.try_get<HitPoint>())
			{
				ent.set<octopus::BuffComponent<BuffType>>({BuffType(), octopus::get_time_stamp(ent.world()), BuffType::DURATION_TICKS, false});
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};
