#pragma once

#include "StatsSet.h"
#include "octopus/utils/FixedPoint.hh"

namespace godoctopus {

struct StatsModifier {
	StatsType type;
	octopus::Fixed base_delta;
	octopus::Fixed coefficients[StatsTypeCount];
}; // struct StatsModifier

octopus::Fixed compute_delta(StatsSet const &stats, octopus::Fixed const &base_delta, octopus::Fixed const coefficients[StatsTypeCount]);
void apply_modifier(StatsSet &stats, StatsModifier const &modifier, uint32_t stack_count = 1);

} // namespace godoctopus
