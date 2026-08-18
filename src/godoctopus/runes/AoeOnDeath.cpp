#include "AoeOnDeath.h"

#include "godoctopus/components/stats/StatsSet.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "godoctopus/runes/helpers/apply_hit_point_delta_area.h"

template<typename Rune, bool heal>
void declare_aoe_on_death_system(flecs::world &ecs, octopus::PositionContext const &ctx) {
	// Register timing tracking component
	ecs.component<Rune>()
		.member("coef", &Rune::coef)
		.member("base", &Rune::base)
		.member("range", &Rune::range);

	ecs.observer<godoctopus::CurrentStats const, octopus::Position const, octopus::Team const, Rune const>()
		.template event<trigger_module::Death>()
		.each([&ctx](flecs::entity e, godoctopus::CurrentStats const &stats_set, octopus::Position const &pos, octopus::Team const &team, Rune const &rune) {
			const octopus::Fixed value = godoctopus::compute_value(stats_set.stats, rune.base, rune.coef);
			if constexpr (heal) {
				apply_hit_point_delta_area(e, pos.pos, team.team, ctx, rune.range, value);
			} else {
				apply_hit_point_delta_area(e, pos.pos, team.team, ctx, rune.range, -value);
			}
		});

	declare_trigger_buff<Rune, false>(ecs);
}

void declare_aoe_on_death_runes(flecs::world &ecs, octopus::PositionContext const &ctx) {
	declare_aoe_on_death_system<DamageOnDeathRune, /* heal = */ false>(ecs, ctx);
	declare_aoe_on_death_system<HealOnDeathRune, /* heal = */ true>(ecs, ctx);
}

void fill_aoe_on_death_info(AoeOnDeathInfo &info, ModRuneData const&data) {
	std::copy(std::begin(data.stats_set_coef.values), std::end(data.stats_set_coef.values), std::begin(info.coef));
	info.base = data.base;
	info.range = data.range;
}
