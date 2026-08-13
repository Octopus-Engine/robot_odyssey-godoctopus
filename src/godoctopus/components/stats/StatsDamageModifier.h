#pragma once

#include "octopus/commands/basic/move/DamageModifier.hh"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "StatsSet.h"

namespace godoctopus {

struct DamageType {
	bool is_mechanical = true;
	bool is_pure = true;
};

struct StatsDamageModifier : public octopus::ArmorDamageModifier {
	octopus::Fixed modify_attack(flecs::entity const &attacker, flecs::entity const &target, octopus::Attack const &attack) const override {
		const auto *attacker_stats = attacker.try_get<CurrentStats>();
		const auto *target_stats = target.try_get<CurrentStats>();
		const auto *attacker_damage_type = attacker.try_get<DamageType>();

		octopus::Fixed damage = octopus::Fixed::Zero();
		if (attacker_stats && target_stats && attacker_damage_type) {
			octopus::Fixed power = attacker_stats->stats.values[StatsType::MechanicalPower];
			octopus::Fixed armor = target_stats->stats.values[StatsType::MechanicalArmor];
			if (!attacker_damage_type->is_mechanical) {
				power = attacker_stats->stats.values[StatsType::PlasmaPower];
				armor = target_stats->stats.values[StatsType::PlasmaArmor];
			}
			const auto delta = power - armor;
			if (delta >= octopus::Fixed::Zero()) {
				damage = attack.cst.damage * (1 + delta/1000);
			} else {
				damage = attack.cst.damage * (1 / (1 - delta/100));
			}
			damage = std::max(octopus::Fixed::One(), damage - target_stats->stats.values[StatsType::Shield]);
		} else {
			damage = octopus::ArmorDamageModifier::modify_attack(attacker, target, attack);
		}
		trigger_module::emit_damage_triggers(attacker, target, damage);
		return damage;
	}
};

} // namespace godoctopus
