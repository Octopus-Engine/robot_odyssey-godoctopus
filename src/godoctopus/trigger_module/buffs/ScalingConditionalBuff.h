#include "ScalingBuff.h"

template<bool under, int64_t percent, int64_t armor_x10_base, int64_t armor_x10_per_special>
struct ConditionalArmorBuffLowLifeRune :
	SpecialScaledBuff<
		ConditionalArmorBuffLowLifeRune<under, percent, armor_x10_base, armor_x10_per_special>,
		LeveledArmorBuff<armor_x10_base, armor_x10_per_special>,
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
};

template<bool under, int64_t percent, int64_t damage_base, int64_t damage_per_special>
struct ConditionalDamageBuffLowLifeRune :
	SpecialScaledBuff<
		ConditionalDamageBuffLowLifeRune<under, percent, damage_base, damage_per_special>,
		LeveledDamageBuff<damage_base, damage_per_special>,
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
};

template<bool under, int64_t percent, int64_t reload_x10_base, int64_t reload_x10_per_special>
struct ConditionalReloadBuffLowLifeRune :
	SpecialScaledBuff<
		ConditionalReloadBuffLowLifeRune<under, percent, reload_x10_base, reload_x10_per_special>,
		LeveledAttackSpeedBuff<reload_x10_base, reload_x10_per_special>,
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
};
