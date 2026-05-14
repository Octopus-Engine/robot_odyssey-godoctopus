#include "UnitProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_unit_producer_beacon_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new UnitProducerBeaconSpawnAbility());

	ecs.component(UnitProducerBeaconSpawnAbility::NAME().c_str());
}
