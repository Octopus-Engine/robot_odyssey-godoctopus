#pragma once

#include "flecs.h"
#include "octopus/world/position/PositionContext.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/attack/Attack.hh"

#include "godoctopus/components/special/Special.h"

template<typename RuneType>
int64_t get_base(RuneType const &rune) {
	return rune.base;
}

template<typename RuneType>
int64_t get_upgrade(RuneType const &rune) {
	return rune.upgrade;
}

template<typename RuneType>
int64_t get_duration_ticks(RuneType const &rune) {
	return rune.duration_ticks;
}

// Base temporary buff event - applies buff to single entity
template<typename RuneType>
struct ApplyTemporaryBuffEvent
{
	static void apply(flecs::entity source, int32_t) {
		using BuffType = typename RuneType::BuffType;
		const auto& rune = source.get<RuneType>();
		// Apply buff directly to the entity
		source.set<octopus::BuffComponent<BuffType>>({BuffType({get_base(rune) + get_special_value(source) * get_upgrade(rune)}), octopus::get_time_stamp(source.world()), get_duration_ticks(rune), false});
	}
};

// Base temporary buff event - applies buff to single entity
template<typename BuffType>
struct ApplyTemporaryBuffComponentEvent
{
	static void apply(flecs::entity source, int32_t) {
		// Apply buff directly to the entity
		source.set<octopus::BuffComponent<BuffType>>({{}, octopus::get_time_stamp(source.world()), BuffType::DURATION_TICKS, false});
	}
};

// Apply a temporary buff to all allies in an area
template<typename RuneType>
struct ApplyTemporaryBuffAreaEvent
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level, int32_t base, int32_t upgrade, int32_t range)
	{
		using BuffType = typename RuneType::BuffType;
		const auto& rune = source.get<RuneType>();
		const auto duration = get_duration_ticks(rune);
		using namespace octopus;

		auto func_l = [team, duration, base, upgrade](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team == team && ent.try_get<HitPoint>())
			{
				ent.set<octopus::BuffComponent<BuffType>>({BuffType({base + get_special_value(ent) * upgrade}), octopus::get_time_stamp(ent.world()), duration, false});
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};

// Apply a temporary debuff to all enemies in an area
template<typename RuneType>
struct ApplyTemporaryDebuffAreaEvent
{
	static void apply(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level, int32_t base, int32_t upgrade, int32_t range)
	{
		using BuffType = typename RuneType::BuffType;
		const auto& rune = source.get<RuneType>();
		const auto duration = get_duration_ticks(rune);
		using namespace octopus;

		auto func_l = [team, duration, base, upgrade](int32_t idx_l, flecs::entity ent) -> bool {
			if(ent.try_get<Team>() && ent.try_get<Team>()->team != team && ent.try_get<HitPoint>())
			{
				ent.set<octopus::BuffComponent<BuffType>>({BuffType({base + get_special_value(ent) * upgrade}), octopus::get_time_stamp(ent.world()), duration, false});
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};
