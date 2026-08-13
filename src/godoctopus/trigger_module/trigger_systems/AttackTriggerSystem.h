#pragma once

#include "flecs.h"

#include "octopus/commands/basic/move/AttackCommandSystem.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "godoctopus/trigger_module/TriggerTypes.h"

void declare_trigger_attack_system(flecs::world &ecs)
{
	using namespace octopus;

	// trigger attack event
	ecs.system<AttackTrigger const>()
		.kind(ecs.entity(EndUpdatePhase))
		.each([&](flecs::entity e, AttackTrigger const& trigger) {
			ecs.event<trigger_module::Attack>()
				.id<AttackTrigger>()
				.entity(e)
				.emit();
		});
}
