#include "LifestealRune.h"

#include "godoctopus/components/stats/StatsSet.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "godoctopus/on_screen_ui/DamageTextNode.h"

void declare_lifesteal_rune(flecs::world &ecs) {
	ecs.component<LifestealRune>()
		.member("coef", &LifestealRune::coef)
		.member("base", &LifestealRune::base)
	;

	ecs.observer<LifestealRune const, godoctopus::CurrentStats const, trigger_module::DamageDealt const, octopus::Attack const, octopus::HitPoint>()
		.template event<trigger_module::DamageDealt>()
		.each([](flecs::entity e, const LifestealRune& rune, godoctopus::CurrentStats const &stats_set, trigger_module::DamageDealt const &damage_dealt, octopus::Attack const &atk, octopus::HitPoint &hp) {
			const auto lifesteal_value = godoctopus::compute_value(stats_set.stats, rune.base, rune.coef) / 100;
			octopus::Fixed const heal = std::max(lifesteal_value, octopus::Fixed::One());
			hp.qty += heal;
			trigger_module::emit_damage_text_trigger(e, heal, true);
		});

	declare_trigger_buff<LifestealRune, false>(ecs);
}

LifestealRune make_lifesteal_rune(ModRuneData const&data) {
	LifestealRune rune;
	std::copy(std::begin(data.stats_set_coef.values), std::end(data.stats_set_coef.values), std::begin(rune.coef));
	rune.base = data.base;
	return rune;
}
