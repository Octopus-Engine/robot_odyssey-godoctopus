#pragma once

#include "octopus/commands/basic/move/DamageModifier.hh"
#include "StatsUpdateSystems.h"

namespace godoctopus {

struct DamageType {
	bool is_mechanical = true;
	bool is_pure = true;
};

struct StatsDamageModifier : public octopus::ArmorDamageModifier {
	octopus::Fixed modify_attack(flecs::entity const &attacker, flecs::entity const &target, octopus::Attack const &attack) const override {
		auto *attacker_stats = attacker.try_get<CurrentStats>();
		auto *target_stats = target.try_get<CurrentStats>();
		auto *attacker_damage_type = attacker.try_get<DamageType>();
		if (attacker_stats && target_stats && attacker_damage_type) {
			auto power = attacker_stats->stats.values[StatsType::MechanicalPower];
			auto armor = target_stats->stats.values[StatsType::MechanicalArmor];
			if (!attacker_damage_type->is_mechanical) {
				power = attacker_stats->stats.values[StatsType::PlasmaPower];
				armor = target_stats->stats.values[StatsType::PlasmaArmor];
			}
			auto delta = power - armor;
			auto damage = octopus::Fixed::Zero();
			if (delta >= octopus::Fixed::Zero()) {
				damage = attack.cst.damage * (1 + delta/100);
			} else {
				damage = attack.cst.damage * (1 / (1 - delta/100));
			}
			return std::max(octopus::Fixed::One(), damage - target_stats->stats.values[StatsType::Shield]);
		}
		return octopus::ArmorDamageModifier::modify_attack(attacker, target, attack);
	}
};

} // namespace godoctopus
