#pragma once

#include "godoctopus/trigger_module/TriggerTypes.h"
#include "octopus/components/basic/hitpoint/Destroyable.hh"

void declare_trigger_life_system(flecs::world &ecs)
{
	using namespace octopus;

	ecs.observer<Destroyable const>()
		.event<Destroyed>()
		.each([&](flecs::entity e, Destroyable const&) {
			ecs.event<trigger_module::Death>()
				.id<Destroyable>()
				.entity(e)
				.emit();
		});
}
