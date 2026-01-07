#pragma once

#include "flecs.h"
#include "octopus/world/position/PositionContext.hh"
#include "octopus/utils/FixedPoint.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"

template<int32_t delta, int32_t upgrade>
struct HitpointEvent
{
	static void apply(flecs::entity target, int32_t level)
	{
		octopus::HitPoint * hp = target.try_get_mut<octopus::HitPoint>();
		hp->qty += delta + upgrade * level;
	}
};

template<int32_t damage, int32_t range, int32_t upgrade>
struct DamageAreaEvent
{
	static void apply(flecs::entity target, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level)
	{
		using namespace octopus;

		auto func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team != team && ent.try_get<HitPoint>())
			{
				HitPoint * hp = ent.try_get_mut<HitPoint>();
				hp->qty -= damage + upgrade * level;
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};

template<int32_t heal, int32_t range, int32_t heal_upgrade>
struct HealAreaEvent
{
	static void apply(flecs::entity target, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level=0)
	{
		using namespace octopus;

		auto func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team == team && ent.try_get<HitPoint>())
			{
				HitPoint * hp = ent.try_get_mut<HitPoint>();
				hp->qty += heal + heal_upgrade * level;
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};
