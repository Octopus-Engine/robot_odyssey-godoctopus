#pragma once

#include "flecs.h"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

/// @brief Checks if an entity's hitpoints are above a specified percentage.
/// @tparam percent The percentage threshold.
template<int32_t percent>
struct HitPointOverPercentCondition
{
	static bool check(flecs::entity e)
	{
		octopus::HitPoint const * hp = e.try_get<octopus::HitPoint>();
		octopus::HitPointMax const * hp_max = e.try_get<octopus::HitPointMax>();
		if(!hp || !hp_max)
		{
			return false;
		}
		octopus::Fixed ratio = octopus::Fixed(percent) / 100;

		return hp->qty / hp_max->qty >= ratio;
	}

	static void post_condition(flecs::entity e) {};
};

/// @brief Checks if an entity's hitpoints are below a specified percentage.
/// @tparam percent The percentage threshold.
template<int32_t percent>
struct HitPointUnderPercentCondition
{
	static bool check(flecs::entity e)
	{
		octopus::HitPoint const * hp = e.try_get<octopus::HitPoint>();
		octopus::HitPointMax const * hp_max = e.try_get<octopus::HitPointMax>();
		if(!hp || !hp_max)
		{
			return false;
		}
		octopus::Fixed ratio = octopus::Fixed(percent) / 100;

		return hp->qty / hp_max->qty <= ratio;
	}
	static void post_condition(flecs::entity e) {};
};

/// @brief Checks if an entity has enough hitpoints for a cost and deducts them if the condition is met.
/// @tparam requirement The amount of hitpoints required.
template<int32_t requirement>
struct HitPointCostCondition
{
	static bool check(flecs::entity e)
	{
		octopus::HitPoint * hp = e.try_get_mut<octopus::HitPoint>();
		if(hp && hp->qty > requirement)
		{
			return true;
		}
		return false;
	}
	static void post_condition(flecs::entity e) {
		octopus::HitPoint * hp = e.try_get_mut<octopus::HitPoint>();
		if(hp && hp->qty > requirement)
		{
			hp->qty -= requirement;
		}
	}
};
