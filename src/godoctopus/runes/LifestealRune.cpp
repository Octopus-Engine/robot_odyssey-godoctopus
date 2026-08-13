#include "LifeStealRune.h"

#include "godoctopus/components/stats/StatsSet.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/trigger_module/TriggerTypes.h"

void declare_lifesteal_rune(flecs::world &ecs) {
	ecs.component<LifestealRune>()
		.member("coef", &LifestealRune::coef)
		.member("base", &LifestealRune::base)
	;
	ecs.observer<LifestealRune const, godoctopus::CurrentStats const, octopus::AttackTrigger const, octopus::Attack const, octopus::HitPoint>()
		.template event<trigger_module::DamageDealt>()
		.each([](flecs::entity e, const LifestealRune& rune, godoctopus::CurrentStats const &stats_set, octopus::AttackTrigger const &trigger, octopus::Attack const &atk, octopus::HitPoint &hp) {
			const octopus::Fixed value = godoctopus::compute_value(stats_set.stats, rune.base, rune.coef);
			hp.qty += std::max(atk.cst.damage * value, octopus::Fixed::One());
		});

	declare_trigger_buff<LifestealRune, false>(ecs);
}

LifestealRune make_lifesteal_rune(ModRuneData const&data) {
	LifestealRune rune;
	std::copy(std::begin(data.stats_set_coef.values), std::end(data.stats_set_coef.values), std::begin(rune.coef));
	rune.base = data.base;
	return rune;
}
