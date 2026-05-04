#pragma once

#include "flecs.h"

/// @brief Represents a condition that is only true if both condition are true.
template<typename CondA, typename CondB>
struct AndCondition
{
	static bool check(flecs::entity e) {
		return CondA::check(e) && CondB::check(e);
	}
	static void post_condition(flecs::entity e) {
		CondA::post_condition(e);
		CondB::post_condition(e);
	};
};
