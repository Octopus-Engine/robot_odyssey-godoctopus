#include "BasicResourceProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

template<typename AbilityT>
void declare_basic_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &, Dictionary const &meta_data)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	auto ability = new AbilityT();
	load_beacon_spawn_ability_meta_data(*ability, meta_data);

	ability_library.add_template(ability);
	ecs.component(AbilityT::NAME().c_str());
}

void declare_basic_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &game, Dictionary const &meta_data)
{
	declare_basic_resource_producer_beacon_ability<BasicResourceProducerBeaconSpawnAbility>(ecs, game, meta_data);
	declare_basic_resource_producer_beacon_ability<BigBasicResourceProducerBeaconSpawnAbility>(ecs, game, meta_data);
}
