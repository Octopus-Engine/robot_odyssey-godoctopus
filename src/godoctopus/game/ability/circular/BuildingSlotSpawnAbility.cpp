#include "BuildingSlotSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_building_slot_spawn_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new BuildingSlotSpawnAbility10_4());
	ability_library.add_template(new BuildingSlotSpawnAbility17_8());
	ability_library.add_template(new BuildingSlotSpawnAbility24_16());

	// Register the ability name as an ECS component so Caster can reference it
	ecs.component(BuildingSlotSpawnAbility10_4::NAME().c_str());
	ecs.component(BuildingSlotSpawnAbility17_8::NAME().c_str());
	ecs.component(BuildingSlotSpawnAbility24_16::NAME().c_str());
}
