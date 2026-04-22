#include "ProximityBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_proximity_beacon_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new ProximityBeaconSpawnAbility());

	ecs.component(ProximityBeaconSpawnAbility::NAME().c_str());
}
