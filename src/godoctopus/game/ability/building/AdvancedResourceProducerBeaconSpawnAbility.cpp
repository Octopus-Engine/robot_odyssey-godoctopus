#include "AdvancedResourceProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_advanced_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &, Dictionary const &meta_data)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	auto ability = new AdvancedResourceProducerBeaconSpawnAbility();
	load_beacon_spawn_ability_meta_data(*ability, meta_data);

	ability_library.add_template(ability);
	ecs.component(AdvancedResourceProducerBeaconSpawnAbility::NAME().c_str());
}
