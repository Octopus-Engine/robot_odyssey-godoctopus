#include "EarbotSteam.h"

void declare_earbot_steam_ability(flecs::world &ecs, octopus::DefaultStepContext<custom_variant>& step_context) {
	// register ability into library
	auto &ability_library = ecs.get_mut<octopus::AbilityTemplateLibrary<custom_step_manager> >();
	ability_library.add_template(new EarbotSteam());

	// register buff component
	ecs.component<EarbotSteamBuff>()
		.member("reload_diminisher", &EarbotSteamBuff::reload_diminisher);

	// register buff effect
	octopus::declare_buff_system<EarbotSteamBuff>(ecs, step_context.step_manager);
	octopus::declare_stats_buff_systems<EarbotSteamBuff, octopus::Attack>(
		ecs,
		[](EarbotSteamBuff const& buff, octopus::Attack &atk) {
			atk.cst.reload_time -= buff.reload_diminisher;
		},
		[](EarbotSteamBuff const& buff, octopus::Attack &atk) {
			atk.cst.reload_time += buff.reload_diminisher;
		}
	);
}
