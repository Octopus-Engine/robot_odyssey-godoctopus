#include "BasicResourceProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_basic_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &, Dictionary const &meta_data)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	auto ability = new BasicResourceProducerBeaconSpawnAbility();

	if (meta_data.has("Abilities")) {
		std::cout<<"ok1"<<std::endl;
		Dictionary abilities = meta_data["Abilities"];
		std::unordered_map<std::string, octopus::Fixed> resource_consumption;
		if (abilities.has(BasicResourceProducerBeaconSpawnAbility::NAME().c_str())) {
		std::cout<<"ok2"<<std::endl;
			Dictionary upgrade_info = abilities[BasicResourceProducerBeaconSpawnAbility::NAME().c_str()];
			if (upgrade_info.has("PlayerResourceConsumption")) {
		std::cout<<"ok3"<<std::endl;
				Dictionary consumption_dict = upgrade_info["PlayerResourceConsumption"];
				Array const &resource_names = consumption_dict.keys();
				for (int j = 0; j < resource_names.size(); ++j) {
					const String resource_name = resource_names[j];
					const octopus::Fixed cost = octopus::Fixed(double(consumption_dict[resource_name]));
					std::cout<<"ok4 "<<resource_name.utf8().get_data()<<" "<<cost<<std::endl;
					resource_consumption[resource_name.utf8().get_data()] = cost;
				}
			}
		}
		ability->player_resource_consumption_cache = resource_consumption;
	}

	ability_library.add_template(ability);
	ecs.component(BasicResourceProducerBeaconSpawnAbility::NAME().c_str());
}
