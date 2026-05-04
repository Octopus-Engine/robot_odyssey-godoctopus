#pragma once

#include "flecs.h"

/// @brief Represents a condition that always evaluates to true.
struct AlwaysCondition
{
	static bool check(flecs::entity) {
		return true;
	}
	static void post_condition(flecs::entity e) {};
};
