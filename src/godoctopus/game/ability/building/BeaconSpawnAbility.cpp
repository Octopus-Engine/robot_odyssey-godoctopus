#include "BeaconSpawnAbility.h"

#include "godoctopus/game/GameNode.h"

void declare_beacon_spawn_ability(flecs::world &ecs, godot::GameNode &)
{
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	ability_library.add_template(new BeaconSpawnAbility());

	// Register the ability name as an ECS component so Caster can reference it
	ecs.component(BeaconSpawnAbility::NAME().c_str());
}

void load_beacon_spawn_ability_meta_data(BeaconSpawnAbility &ability, Dictionary const &meta_data) {

	if (meta_data.has("Abilities")) {
		Dictionary abilities = meta_data["Abilities"];
		std::unordered_map<std::string, octopus::Fixed> resource_consumption;
		if (abilities.has(ability.name().c_str())) {
			Dictionary upgrade_info = abilities[ability.name().c_str()];
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
		ability.player_resource_consumption_cache = resource_consumption;
	}
}
