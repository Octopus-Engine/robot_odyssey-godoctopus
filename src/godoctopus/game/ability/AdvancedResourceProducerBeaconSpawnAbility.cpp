#include "AdvancedResourceProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_advanced_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new AdvancedResourceProducerBeaconSpawnAbility());

	ecs.component(AdvancedResourceProducerBeaconSpawnAbility::NAME().c_str());
}
