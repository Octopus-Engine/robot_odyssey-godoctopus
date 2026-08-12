#include "StatsModifier.h"

namespace godoctopus {

void apply_modifier(StatsSet &stats, StatsModifier const &modifier, uint32_t stack_count) {
	stats.values[modifier.type] += compute_delta(stats, modifier.base_delta, modifier.coefficients) * stack_count;
}

} // namespace godoctopus
