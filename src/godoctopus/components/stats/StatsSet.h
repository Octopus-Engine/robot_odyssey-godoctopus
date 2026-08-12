#pragma once

#include "octopus/utils/FixedPoint.hh"

namespace godoctopus {

enum StatsType {
	HitPoints = 0u,
	Damage,
	Shield,
	MechanicalPower,
	MechanicalArmor,
	PlasmaPower,
	PlasmaArmor,
	Speed,
	Affinity,
	StatsTypeCount
};

struct StatsSet {
	octopus::Fixed values[StatsTypeCount];
};

struct BaseStats {
	StatsSet stats;
};

struct CurrentStats {
	StatsSet stats;
};

octopus::Fixed compute_delta(StatsSet const &stats, octopus::Fixed const &base_delta, octopus::Fixed const coefficients[StatsTypeCount]);

} // godoctopus
