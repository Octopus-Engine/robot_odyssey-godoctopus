#include "AoePulseRune.h"

#include "godoctopus/components/stats/StatsSet.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/runes/helpers/apply_hit_point_delta_area.h"

/// @brief Tracking component for periodic trigger timing
template<typename Rune>
struct PulseRuneTriggerTime
{
	int64_t last_trigger_time = -TICK_RATE;  // Initialize to trigger on first check
};

/// @brief Periodic trigger system that applies an AoE effect every N ticks
/// Requires the entity to have the Rune component, and will check conditions and apply events on intervals
template<typename Rune, bool heal>
void declare_periodic_area_trigger_system(flecs::world &ecs, octopus::PositionContext const &ctx, int64_t tick_interval = TICK_RATE) {
	// Register timing tracking component
	ecs.component<Rune>()
		.member("coef", &Rune::coef)
		.member("base", &Rune::base)
		.member("range", &Rune::range)
	;
	ecs.component<PulseRuneTriggerTime<Rune>>();

	ecs.system<Rune, godoctopus::CurrentStats const, octopus::Position const, octopus::Team const, PulseRuneTriggerTime<Rune> *>()
		.kind(ecs.entity(UpdatePhase))
		.each([&ctx, tick_interval](flecs::entity e, Rune const &rune, godoctopus::CurrentStats const &stats_set, octopus::Position const &pos, octopus::Team const &team, PulseRuneTriggerTime<Rune> *trigger_time) {
			int64_t current_time = octopus::get_time_stamp(e.world());
			const octopus::Fixed value = godoctopus::compute_value(stats_set.stats, rune.base, rune.coef);

			// Check if enough ticks have passed since last trigger
			if(!trigger_time || current_time - trigger_time->last_trigger_time >= tick_interval) {
				if(trigger_time) {
					trigger_time->last_trigger_time = current_time;
				} else {
					e.set<PulseRuneTriggerTime<Rune>>({current_time});
				}
				if (heal) {
					apply_hit_point_delta_area(e, pos.pos, team.team, ctx, rune.range, value);
				} else {
					apply_hit_point_delta_area(e, pos.pos, team.team, ctx, rune.range, -value);
				}
			}
		});

	declare_trigger_buff<Rune, false>(ecs);
}

void declare_aoe_pulse_runes(flecs::world &ecs, octopus::PositionContext const &ctx) {
	declare_periodic_area_trigger_system<AoePulseDamages, /* heal = */ false>(ecs, ctx);
}

void fill_aoe_pulse_info(AoePulseInfo &info, ModRuneData const&data) {
	std::copy(std::begin(data.stats_set_coef.values), std::end(data.stats_set_coef.values), std::begin(info.coef));
	info.base = data.base;
	info.range = data.range;
}
