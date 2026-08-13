#include "StatsSet.h"

namespace godoctopus {

octopus::Fixed compute_value(StatsSet const &stats, octopus::Fixed const &base_value, octopus::Fixed const coefficients[StatsTypeCount]) {
	octopus::Fixed value = base_value;
	for (size_t i = 0; i < StatsType::StatsTypeCount; ++i) {
		value += stats.values[i] * coefficients[i];
	}
	return value;
}

} // namespace godoctopus
