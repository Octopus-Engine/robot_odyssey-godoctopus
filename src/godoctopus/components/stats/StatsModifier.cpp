#include "StatsModifier.h"

namespace godoctopus {

octopus::Fixed compute_delta(StatsSet const &stats, octopus::Fixed const &base_delta, octopus::Fixed const coefficients[StatsTypeCount]) {
	octopus::Fixed delta = base_delta;
	for (size_t i = 0; i < StatsType::StatsTypeCount; ++i) {
		delta += stats.values[i] * coefficients[i];
	}
	return delta;
}

void apply_modifier(StatsSet &stats, StatsModifier const &modifier, uint32_t stack_count) {
	stats.values[modifier.type] += compute_delta(stats, modifier.base_delta, modifier.coefficients) * stack_count;
}

} // namespace godoctopus
