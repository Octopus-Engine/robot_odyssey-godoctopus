#include "CircularSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_circular_spawn_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	// Note: CircularSpawnAbility is abstract, so no direct instantiation here
	// Only concrete subclasses are registered

	// Register the base ability name as an ECS component
	ecs.component(CircularSpawnAbility::NAME().c_str());
}
