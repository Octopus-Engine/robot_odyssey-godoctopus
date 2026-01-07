#pragma once

#include "flecs.h"

/// @brief Represents a condition that always evaluates to true.
struct AlwaysCondition
{
	static bool check(flecs::entity)
	{
		return true;
	}
};
