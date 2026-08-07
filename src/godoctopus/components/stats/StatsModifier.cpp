#include "StatsModifier.h"

namespace godoctopus {

octopus::Fixed compute_delta(StatsSet const &stats, StatsModifier const &modifier) {
	octopus::Fixed delta = modifier.base_delta;
	for (size_t i = 0; i < StatsType::StatsTypeCount; ++i) {
		delta += stats.values[i] * modifier.coefficients[i];
	}
	return delta;
}

void apply_modifier(StatsSet &stats, StatsModifier const &modifier, uint32_t stack_count) {
	std::cout<<"Applying modifier of type " << modifier.type << " with base " << modifier.base_delta.to_int() << std::endl;
	stats.values[modifier.type] += compute_delta(stats, modifier) * stack_count;
}

} // namespace godoctopus
