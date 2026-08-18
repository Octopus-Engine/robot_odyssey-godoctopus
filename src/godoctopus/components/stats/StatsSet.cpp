#include "StatsSet.h"

namespace godoctopus {

octopus::Fixed compute_value(StatsSet const &stats, octopus::Fixed const &base_value, octopus::Fixed const coefficients[StatsTypeCount]) {
	octopus::Fixed value = base_value;
	for (size_t i = 0; i < StatsType::StatsTypeCount; ++i) {
		value += stats.values[i] * coefficients[i];
	}
	return value;
}

StatsType type_from_string(std::string const &str) {
	if (str == "HitPoints") {
		return StatsType::HitPoints;
	}
	else if (str == "Damage") {
		return StatsType::Damage;
	}
	else if (str == "Shield") {
		return StatsType::Shield;
	}
	else if (str == "MechanicalPower") {
		return StatsType::MechanicalPower;
	}
	else if (str == "MechanicalArmor") {
		return StatsType::MechanicalArmor;
	}
	else if (str == "PlasmaPower") {
		return StatsType::PlasmaPower;
	}
	else if (str == "PlasmaArmor") {
		return StatsType::PlasmaArmor;
	}
	else if (str == "Speed") {
		return StatsType::Speed;
	}
	else if (str == "Affinity") {
		return StatsType::Affinity;
	}
	return StatsType::HitPoints;
}

} // namespace godoctopus
