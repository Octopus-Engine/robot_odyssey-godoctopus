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

} // godoctopus
