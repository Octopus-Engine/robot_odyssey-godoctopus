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

	// trigger simple damage dealt
	ecs.system<AttackTrigger const>()
		.kind(ecs.entity(EndUpdatePhase))
		.without<NoInstantDamage>()
		.each([&](flecs::entity e, AttackTrigger const& trigger) {
			//dealt
			ecs.event<trigger_module::DamageDealt>()
				.id<AttackTrigger>()
				.entity(e)
				.emit();
		});

	// trigger simple damage received event
	ecs.system<AttackTrigger const>()
		.kind(ecs.entity(EndUpdatePhase))
		.without<NoInstantDamage>()
		.each([&](flecs::entity e, AttackTrigger const& trigger) {
			// received
			ecs.event<trigger_module::DamageReceived>()
				.id<HitPoint>()
				.entity(trigger.target)
				.emit();
		});
}
