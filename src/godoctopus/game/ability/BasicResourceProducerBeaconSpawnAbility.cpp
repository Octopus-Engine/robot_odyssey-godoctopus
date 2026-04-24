#include "BasicResourceProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_basic_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new BasicResourceProducerBeaconSpawnAbility());

	ecs.component(BasicResourceProducerBeaconSpawnAbility::NAME().c_str());
}
