#pragma once

#include "flecs.h"
#include "octopus/world/position/PositionContext.hh"
#include "octopus/utils/FixedPoint.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "godoctopus/components/special/Special.h"

template<int32_t delta, int32_t upgrade>
struct HitpointEvent
{
	static void apply(flecs::entity source, int32_t level)
	{
		octopus::HitPoint * hp = source.try_get_mut<octopus::HitPoint>();
		hp->qty += delta + upgrade * level;
	}
};

template<int32_t damage, int32_t range, int32_t upgrade>
struct DamageAreaEvent
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level)
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

template<int32_t damage, int32_t range, int32_t upgrade>
struct DamageAreaEventSpecialScaled
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t)
	{
		using namespace octopus;

		auto func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team != team && ent.try_get<HitPoint>())
			{
				HitPoint * hp = ent.try_get_mut<HitPoint>();
				hp->qty -= damage + upgrade * get_special_value(source);
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};

template<int32_t damage, int32_t range, int32_t upgrade>
struct DamageAreaEventHitpointBasedSpecialScaled
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t)
	{
		using namespace octopus;
		auto const special = get_special_value(source);
		auto const &hp_max = source.get<octopus::HitPointMax>();
		auto const aoe_dmg = hp_max.qty * (special * upgrade + damage) / 100;

		auto func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team != team && ent.try_get<HitPoint>())
			{
				HitPoint * hp = ent.try_get_mut<HitPoint>();
				hp->qty -= aoe_dmg;
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};

template<int32_t heal, int32_t range, int32_t heal_upgrade>
struct HealAreaEvent
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level=0)
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

template<int32_t heal, int32_t range, int32_t heal_upgrade>
struct HealAreaEventSpecialScaled
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t)
	{
		using namespace octopus;

		auto func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team == team && ent.try_get<HitPoint>())
			{
				HitPoint * hp = ent.try_get_mut<HitPoint>();
				hp->qty += heal + heal_upgrade * get_special_value(source);
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};
