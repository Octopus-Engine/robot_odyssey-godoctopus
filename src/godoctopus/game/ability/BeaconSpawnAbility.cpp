#include "BeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_beacon_spawn_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new BeaconSpawnAbility());

	// Register the ability name as an ECS component so Caster can reference it
	ecs.component(BeaconSpawnAbility::NAME().c_str());
}
