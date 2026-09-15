#include "AoeOnDeath.h"

#include "octopus/components/basic/hitpoint/Destroyable.hh"
#include "godoctopus/components/stats/StatsSet.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "godoctopus/runes/helpers/apply_hit_point_delta_area.h"

template<typename Rune, bool heal>
void declare_aoe_on_death_system(flecs::world &ecs, octopus::PositionContext const &ctx, godot::ParticleLibrary *particle_library) {
	// Register timing tracking component
	ecs.component<Rune>()
		.member("coef", &Rune::coef)
		.member("base", &Rune::base)
		.member("range", &Rune::range);

	ecs.observer<octopus::Destroyable const, godoctopus::CurrentStats const, octopus::Position const, octopus::Team const,
				Rune const, octopus::Collision const>()
		.template event<trigger_module::Death>()
		.each([&ctx, particle_library](flecs::entity e, octopus::Destroyable const&, godoctopus::CurrentStats const &stats_set, octopus::Position const &pos,
				octopus::Team const &team, Rune const &rune, octopus::Collision const &col) {
			const octopus::Fixed value = godoctopus::compute_value(stats_set.stats, rune.base, rune.coef);
			if constexpr (heal) {
				apply_hit_point_delta_area(e, pos.pos, team.team, ctx, rune.range, value);
			} else {
				apply_hit_point_delta_area(e, pos.pos, team.team, ctx, rune.range, -value);
			}
			if (particle_library && particle_library->get_element_from_key("SphereImpact")) {
				Vector3 effect_position = WORLD_SCALE * Vector3(pos.pos.x.to_double(), 0., pos.pos.y.to_double());
				particle_library->get_element_from_key("SphereImpact")->add_instance_detailed(effect_position, Color(2.454, 0.682, 2.268, 1.0), 1, Vector3(3.,3.,3.) * octopus::to_double(col.ray), 0);
			}
		});

	declare_trigger_buff<Rune, false>(ecs);
}

void declare_aoe_on_death_runes(flecs::world &ecs, octopus::PositionContext const &ctx, godot::ParticleLibrary *particle_library) {
	declare_aoe_on_death_system<DamageOnDeathRune, /* heal = */ false>(ecs, ctx, particle_library);
	declare_aoe_on_death_system<HealOnDeathRune, /* heal = */ true>(ecs, ctx, particle_library);
}

void fill_aoe_on_death_info(AoeOnDeathInfo &info, ModRuneData const&data) {
	std::copy(std::begin(data.stats_set_coef.values), std::end(data.stats_set_coef.values), std::begin(info.coef));
	info.base = data.base;
	info.range = data.range;
}
