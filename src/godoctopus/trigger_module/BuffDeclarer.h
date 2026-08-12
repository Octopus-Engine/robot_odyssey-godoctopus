#pragma once

#include "flecs.h"

#include "octopus_types.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"
#include "godoctopus/components/types/Types.h"

template<typename BuffType, typename... ComponentType>
struct BuffDeclarer {
	flecs::world &ecs;
	bool add_debuff_all_system;

	template<typename BotType>
	void operator()() const {
		octopus::declare_player_buff_systems<BotType, BuffType, ComponentType...>(ecs, add_debuff_all_system);
	}
};

template<typename BuffType, typename... ComponentType>
void declare_player_buff_systems_all_units(flecs::world &ecs, bool add_debuff_all_system)
{
	for_each_bot_type(BuffDeclarer<BuffType, ComponentType...>{ecs, add_debuff_all_system});
}

template<typename Trigger, bool declare_component = true>
void declare_trigger_buff(flecs::world &ecs)
{
	// component declaration
	if constexpr (declare_component) {
		ecs.component<Trigger>()
			.member("level", &Trigger::level)
		;
	}
	ecs.component<octopus::BuffAddComponent<Trigger>>()
		.member("placeholder", &octopus::BuffAddComponent<Trigger>::placeholder)
	;

	declare_player_buff_systems_all_units<typename octopus::BuffAddComponent<Trigger>>(ecs, false);
}
