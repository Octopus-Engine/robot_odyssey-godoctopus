#pragma once

template<int32_t percent, int32_t percent_upgrade>
struct FactorDamageEvent
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t level)
	{
		octopus::HitPoint * hp = target.try_get_mut<octopus::HitPoint>();
		octopus::Attack const *atk = attacker.try_get<octopus::Attack>();
		if(hp && atk)
		{
			hp->qty -= atk->cst.damage * (percent + percent_upgrade * level)/100;
		}
	}
};
