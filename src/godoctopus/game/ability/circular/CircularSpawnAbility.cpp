#include "CircularSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_circular_spawn_ability(flecs::world &ecs, godot::GameNode &)
{
	// Note: CircularSpawnAbility is abstract, so no direct instantiation here
	// Only concrete subclasses are registered

	// Register the base ability name as an ECS component
	ecs.component(CircularSpawnAbility::NAME().c_str());
}
