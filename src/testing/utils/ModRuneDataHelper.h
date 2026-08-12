#pragma once

#include "godoctopus/trigger_module/TriggerDeclaration.h"

inline Dictionary create_rune_data(int level = 1, int flat_buff = 0, int base = 0, int level_upgrade = 0, int upgrade = 0, int range = 0, int duration_ticks = 0) {
	Dictionary rune_data;
	rune_data["level"] = level;
	rune_data["flat_buff"] = flat_buff;
	rune_data["base"] = base;
	rune_data["level_upgrade"] = level_upgrade;
	rune_data["upgrade"] = upgrade;
	rune_data["range"] = range;
	rune_data["duration_ticks"] = duration_ticks;
	return rune_data;
}

struct RuneDataBuilder {

	RuneDataBuilder &set_level(int level) {
		rune_data["level"] = level;
		return *this;
	}

	RuneDataBuilder &set_range(int level) {
		rune_data["range"] = level;
		return *this;
	}

	RuneDataBuilder &set_base(int base) {
		rune_data["base"] = base;
		return *this;
	}

	RuneDataBuilder &setHitPointsCoefPercent(int percent) {
		rune_data["HitPointsCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setDamageCoefPercent(int percent) {
		rune_data["DamageCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setShieldCoefPercent(int percent) {
		rune_data["ShieldCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setMechanicalPowerCoefPercent(int percent) {
		rune_data["MechanicalPowerCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setMechanicalArmorCoefPercent(int percent) {
		rune_data["MechanicalArmorCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setPlasmaPowerCoefPercent(int percent) {
		rune_data["PlasmaPowerCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setPlasmaArmorCoefPercent(int percent) {
		rune_data["PlasmaArmorCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setSpeedCoefPercent(int percent) {
		rune_data["SpeedCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setAffinityCoefPercent(int percent) {
		rune_data["AffinityCoefPercent"] = percent;
		return *this;
	}
	RuneDataBuilder &setType(godoctopus::StatsType type) {
		rune_data["Type"] = static_cast<int>(type);
		return *this;
	}
	RuneDataBuilder &setModifierPriority(int prio) {
		rune_data["ModifierPriority"] = prio;
		return *this;
	}

	Dictionary build() {
		return rune_data;
	}

private:
	Dictionary rune_data;
};