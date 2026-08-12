#pragma once

#include "octopus/components/advanced/player/buff/PlayerBuff.hh"
#include "godoctopus/components/stats/StatsModifierRecorder.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"

template<int RunesSlot>
struct RuneStats {bool placeholder = false; };

using RuneStats1 = RuneStats<1>;
using RuneStats2 = RuneStats<2>;
using RuneStats3 = RuneStats<3>;
using RuneStats4 = RuneStats<4>;
using RuneStats5 = RuneStats<5>;
using RuneStats6 = RuneStats<6>;
using RuneStats7 = RuneStats<7>;
using RuneStats8 = RuneStats<8>;
using RuneStats9 = RuneStats<9>;

template<int RuneSlot>
void declare_stats_modifiers_rune(flecs::world &ecs) {
	using RuneStatsType = RuneStats<RuneSlot>;
	ecs.component<RuneStatsType>()
		.member("placeholder", &RuneStatsType::placeholder)
	;
	godoctopus::declare_modifier_register<RuneStatsType>(ecs);

	ecs.component<octopus::BuffAddComponent<godoctopus::StatsModifierRecorder<RuneStatsType>>>()
		.member("placeholder", &octopus::BuffAddComponent<godoctopus::StatsModifierRecorder<RuneStatsType>>::placeholder)
	;
	declare_player_buff_systems_all_units<octopus::BuffAddComponent<godoctopus::StatsModifierRecorder<RuneStatsType>>>(ecs, false);
}

inline void declare_stats_modifiers_rune(flecs::world &ecs) {
	declare_stats_modifiers_rune<1>(ecs);
	declare_stats_modifiers_rune<2>(ecs);
	declare_stats_modifiers_rune<3>(ecs);
	declare_stats_modifiers_rune<4>(ecs);
	declare_stats_modifiers_rune<5>(ecs);
	declare_stats_modifiers_rune<6>(ecs);
	declare_stats_modifiers_rune<7>(ecs);
	declare_stats_modifiers_rune<8>(ecs);
	declare_stats_modifiers_rune<9>(ecs);
}
