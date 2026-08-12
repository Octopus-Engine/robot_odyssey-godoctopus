#pragma once

#include "StatsSet.h"
#include "octopus/utils/FixedPoint.hh"

namespace godoctopus {

struct StatsModifier {
	StatsType type;
	octopus::Fixed base_delta;
	octopus::Fixed coefficients[StatsTypeCount];
}; // struct StatsModifier

void apply_modifier(StatsSet &stats, StatsModifier const &modifier, uint32_t stack_count = 1);

} // namespace godoctopus
