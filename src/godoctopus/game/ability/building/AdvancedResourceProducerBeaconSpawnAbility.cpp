#include "AdvancedResourceProducerBeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_advanced_resource_producer_beacon_ability(flecs::world &ecs, godot::GameNode &, Dictionary const &meta_data)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	auto ability = new AdvancedResourceProducerBeaconSpawnAbility();

	if (meta_data.has("Abilities")) {
		Dictionary player_upgrades = meta_data["Abilities"];
		std::unordered_map<std::string, octopus::Fixed> resource_consumption;
		if (player_upgrades.has(AdvancedResourceProducerBeaconSpawnAbility::NAME().c_str())) {
			Dictionary upgrade_info = player_upgrades[AdvancedResourceProducerBeaconSpawnAbility::NAME().c_str()];
			if (upgrade_info.has("PlayerResourceConsumption")) {
				Dictionary consumption_dict = upgrade_info["PlayerResourceConsumption"];
				Array const &resource_names = consumption_dict.keys();
				for (int j = 0; j < resource_names.size(); ++j) {
					const String resource_name = resource_names[j];
					const octopus::Fixed cost = octopus::Fixed(double(consumption_dict[resource_name]));
					resource_consumption[resource_name.utf8().get_data()] = cost;
				}
			}
		}
		ability->player_resource_consumption_cache = resource_consumption;
	}

	ability_library.add_template(ability);
	ecs.component(AdvancedResourceProducerBeaconSpawnAbility::NAME().c_str());
}
