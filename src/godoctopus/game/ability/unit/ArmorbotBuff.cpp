#include "ArmorbotBuff.h"

#include "octopus/components/basic/armor/Armor.hh"

#include "godoctopus/game/GameNode.h"

void declare_armorbot_ability(flecs::world &ecs, godot::GameNode &game) {
	// register ability into library
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager> >();
	ability_library.add_template(new ArmorbotAbility(game.get_world().position_context));

	// register buff component
	ecs.component<ArmorbotBuff>()
		.member("buff", &ArmorbotBuff::buff);

	// register buff effect
	octopus::declare_buff_system<ArmorbotBuff>(ecs, game.get_step_context().step_manager);
	octopus::declare_stats_buff_systems<ArmorbotBuff, octopus::Armor>(
		ecs,
		[](ArmorbotBuff const& buff, octopus::Armor &armor) {
			armor.qty += buff.buff;
		},
		[](ArmorbotBuff const& buff, octopus::Armor &armor) {
			armor.qty -= buff.buff;
		}
	);
}
