#pragma once

#include "ScalingBuff.h"
#include "StatBuff.h"

template<typename RuneType, bool under>
struct ConditionalArmorBuffLowLifeRune :
	SpecialScaledBuff<
		RuneType,
		LeveledArmorBuff,
		octopus::Armor, octopus::HitPoint, octopus::HitPointMax
	> {

	bool is_active(flecs::entity e, octopus::Armor &, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const override {
		if (under) {
			return hp.qty * 100 < hp_max.qty * percent;
		} else {
			return hp.qty * 100 >= hp_max.qty * percent;
		}
	}
	int32_t level = 0;
	int64_t percent = 0;
};

template<typename RuneType, bool under>
struct ConditionalDamageBuffLowLifeRune :
	SpecialScaledBuff<
		RuneType,
		LeveledDamageBuff,
		octopus::Attack, octopus::HitPoint, octopus::HitPointMax
	> {

	bool is_active(flecs::entity e, octopus::Attack &, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const override {
		if (under) {
			return hp.qty * 100 < hp_max.qty * percent;
		} else {
			return hp.qty * 100 >= hp_max.qty * percent;
		}
	}
	int32_t level = 0;
	int64_t percent = 0;
};

template<typename RuneType, bool under>
struct ConditionalReloadBuffLowLifeRune :
	SpecialScaledBuff<
		RuneType,
		LeveledAttackSpeedBuff,
		octopus::Attack, octopus::HitPoint, octopus::HitPointMax
	> {

	bool is_active(flecs::entity e, octopus::Attack &, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const override {
		if (under) {
			return hp.qty * 100 < hp_max.qty * percent;
		} else {
			return hp.qty * 100 >= hp_max.qty * percent;
		}
	}
	int32_t level = 0;
	int64_t percent = 0;
};

template<typename RuneType, godoctopus::StatsType StatsType, bool under>
struct ConditionalStatBuffLowLifeRune :
	SpecialScaledBuff<
		RuneType,
		LeveledStatBuff<StatsType>,
		godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax
	> {

	bool is_active(flecs::entity e, godoctopus::BaseStats &, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const override {
		if (under) {
			return hp.qty * 100 < hp_max.qty * percent;
		} else {
			return hp.qty * 100 >= hp_max.qty * percent;
		}
	}
	int32_t level = 0;
	int64_t percent = 0;
};
