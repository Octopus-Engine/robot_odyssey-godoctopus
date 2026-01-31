#pragma once

#include "godoctopus/components/special/Special.h"

template<int32_t percent, int32_t upgrade_percent>
struct LifestealEvent
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t level=0)
	{
		octopus::HitPoint * hp = attacker.try_get_mut<octopus::HitPoint>();
		octopus::HitPoint const * target_hp = target.try_get<octopus::HitPoint>();
		octopus::Attack const *atk = attacker.try_get<octopus::Attack>();
		if(hp && atk && target_hp)
		{
			int32_t total_percent = percent + upgrade_percent * level;
			hp->qty += std::min(std::max(atk->cst.damage * total_percent/100, octopus::Fixed::One()), target_hp->qty);
		}
	}
};

template<int32_t percent, int32_t upgrade_percent>
struct LifestealEventSpecialScaled
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t)
	{
		octopus::HitPoint * hp = attacker.try_get_mut<octopus::HitPoint>();
		octopus::HitPoint const * target_hp = target.try_get<octopus::HitPoint>();
		octopus::Attack const *atk = attacker.try_get<octopus::Attack>();
		if(hp && atk && target_hp)
		{
			octopus::Fixed total_percent = get_special_value(attacker) * upgrade_percent + percent;
			hp->qty += std::min(std::max(atk->cst.damage * total_percent/100, octopus::Fixed::One()), target_hp->qty);
		}
	}
};
