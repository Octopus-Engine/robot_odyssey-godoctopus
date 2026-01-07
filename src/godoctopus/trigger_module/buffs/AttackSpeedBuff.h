#pragma once
#include "octopus/components/basic/attack/Attack.hh"

template<int32_t base_quantity>
struct AttackSpeedBuff
{
	int32_t quantity = base_quantity;

	void apply(flecs::entity, octopus::Attack &atk) const
	{
		atk.cst.reload_time -= quantity;
	}

	void revert(flecs::entity, octopus::Attack &atk) const
	{
		atk.cst.reload_time += quantity;
	}
};
