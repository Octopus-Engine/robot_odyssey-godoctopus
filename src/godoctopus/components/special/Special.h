#pragma once

#include "flecs.h"
#include "octopus/utils/FixedPoint.hh"

/// @brief A special value with an affinity multiplier
/// than increased the power of runes or abilities
struct Special {
	octopus::Fixed value = octopus::Fixed(0);
	octopus::Fixed affinity = octopus::Fixed::One();
};

octopus::Fixed get_special_value(flecs::entity e);
