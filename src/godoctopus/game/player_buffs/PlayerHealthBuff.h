#pragma once

#include "flecs.h"

#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/utils/FixedPoint.hh"
#include "octopus/systems/player/buff/PlayerBuffSystems.hh"

#include "godoctopus/components/building/Building.h"

struct PlayerHealthBuff {
	octopus::Fixed hp_bonus = 0;

	void apply(flecs::entity e, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty += hp_bonus;
		hp_max.qty += hp_bonus;
	}

	void revert(flecs::entity e, octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty -= hp_bonus;
		hp_max.qty -= hp_bonus;
	}
};

inline void declare_health_buff_systems(flecs::world &ecs) {
	ecs.component<PlayerHealthBuff>()
		.member("hp_bonus", &PlayerHealthBuff::hp_bonus);
	octopus::declare_player_buff_systems<Unit, PlayerHealthBuff, octopus::HitPoint, octopus::HitPointMax>(ecs);
}
