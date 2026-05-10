#pragma once

#include "flecs.h"
#include "octopus/components/basic/timestamp/TimeStamp.hh"

struct DefaultRune {};

template<typename type>
struct RuneLoad
{
	int32_t qty = 0;
	int64_t last_update_time = 0;
};

template<typename type>
void declare_rune_load_component(flecs::world &ecs)
{
	ecs.component<RuneLoad<type>>()
        .member("qty", &RuneLoad<type>::qty)
	;

	ecs.system<RuneLoad<type>>()
		.kind(ecs.entity(ValidatePhase))
		.each([ecs](flecs::entity e, RuneLoad<type> &rune_load) {
			// reset rune after 15 seconds
			if (octopus::get_time_stamp(ecs) - rune_load.last_update_time > 15 * TICK_RATE) {
				rune_load.qty = 0;
			}
			if (rune_load.qty > 5) {
				rune_load.qty = 5;
			}
	});
}
