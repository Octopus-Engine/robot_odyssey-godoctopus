#pragma once

#include "flecs.h"

#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/utils/FixedPoint.hh"
#include "octopus/systems/player/buff/PlayerBuffSystems.hh"

#include "godoctopus/components/building/Building.h"

struct PlayerDamageBuff {
	octopus::Fixed damage_bonus = 0;

	void apply(flecs::entity e, octopus::Attack &attack) const
	{
		attack.cst.damage += damage_bonus;
	}

	void revert(flecs::entity e, octopus::Attack &attack) const
	{
		attack.cst.damage -= damage_bonus;
	}
};

inline void declare_damage_buff_systems(flecs::world &ecs) {
	ecs.component<PlayerDamageBuff>()
		.member("damage_bonus", &PlayerDamageBuff::damage_bonus);
	octopus::declare_player_buff_systems<Unit, PlayerDamageBuff, octopus::Attack>(ecs);
}
