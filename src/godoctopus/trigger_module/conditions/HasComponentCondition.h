#pragma once

#include "flecs.h"

/// @brief Represents a condition that is true if a component is present on the entity.
template<typename Component, bool neg=false>
struct HasComponentCondition {
	static bool check(flecs::entity e) {
		// return true iif the component is present iif the negation is not present
		return (e.try_get<Component>() == nullptr) == neg;
	}
	static void post_condition(flecs::entity e) {};
};
