#include "TriggerDeclaration.h"

#include <algorithm>
#include <iterator>

#include "flecs.h"
#include "core/variant/variant.h"
#include "core/string/print_string.h"

#include "octopus/systems/player/buff/PlayerBuffSystems.hh"

#include "godoctopus/components/types/Types.h"
#include "godoctopus/components/stats/StatsUpdateSystems.h"
#include "godoctopus/components/stats/StatsModifierRecorder.h"
#include "godoctopus/trigger_module/TemporaryBuffTriggerDeclaration.h"
#include "godoctopus/trigger_module/SpawnUnitRune.h"
#include "godoctopus/trigger_module/UndyingRune.h"

#include "godoctopus/runes/AoePulseRune.h"
#include "godoctopus/runes/LifestealRune.h"
#include "godoctopus/runes/AoeOnDeath.h"

#include "StatsModifiersRuneDeclaration.h"

template<typename RuneType, typename... ComponentType>
struct ModRune {
	flecs::entity e;
	bool add;
	std::string const &type;
	RuneType rune;

	template<typename BotType>
	void operator()() const {
		if(type == BotType::naming()) {
			e.world().defer_suspend();
			e.remove<octopus::PlayerBuff<BotType, RuneType, ComponentType...>>();
			if(add) {
				e.set<octopus::PlayerBuff<BotType, RuneType, ComponentType...>>({rune});
			}
			e.world().defer_resume();
		}
	}
};

template<typename RuneType, typename... ComponentType>
void mod_rune_type_add_component(flecs::entity e, bool add, std::string const &type, RuneType &&rune)
{
	using BuffAddComponentT = octopus::BuffAddComponent<RuneType>;
	for_each_bot_type(ModRune<BuffAddComponentT, ComponentType...>{e, add, type, BuffAddComponentT{{std::move(rune)}}});
}

template<typename RuneType, typename... ComponentType>
void mod_rune_type(flecs::entity e, bool add, std::string const &type, RuneType &&rune)
{
	for_each_bot_type(ModRune<RuneType, ComponentType...>{e, add, type, std::forward<RuneType>(rune)});
}

template<typename RuneType>
RuneType makeFlatBuff(int flat_buff) {
	RuneType rune;
	rune.qty = flat_buff;
	return rune;
}

template<typename RuneType>
godoctopus::StatsModifierRecorder<RuneType> makeStatsModifierRecorder(int base, ModRuneData const &data) {
	godoctopus::StatsModifierRecorder<RuneType> rune;
	rune.modifier.base_delta = base;
	std::copy(std::begin(data.stats_set_coef.values), std::end(data.stats_set_coef.values), std::begin(rune.modifier.coefficients));
	rune.modifier.type = data.stats_type;
	rune.list_idx = data.modifier_priority;
	return rune;
}

template<typename RuneType>
RuneType makeScaling(int base, int upgrade) {
	RuneType rune;
	rune.base = base;
	rune.upgrade = upgrade;
	return rune;
}

template<typename RuneType>
RuneType makeScalingPercent(int base, int upgrade, int percent) {
	RuneType rune = makeScaling<RuneType>(base, upgrade);
	rune.percent = percent;
	return rune;
}

template<typename RuneType>
RuneType makeScalingRange(int base, int upgrade, int range) {
	RuneType rune = makeScaling<RuneType>(base, upgrade);
	rune.range = range;
	return rune;
}

template<typename RuneType>
RuneType makeScalingDurationTicks(int base, int upgrade, int duration_ticks	) {
	RuneType rune = makeScaling<RuneType>(base, upgrade);
	rune.duration_ticks = duration_ticks;
	return rune;
}

template<typename RuneType>
RuneType makeScalingRangeDurationTicks(int base, int upgrade, int range, int duration_ticks) {
	RuneType rune = makeScalingRange<RuneType>(base, upgrade, range);
	rune.duration_ticks = duration_ticks;
	return rune;
}

void mod_rune_based_on_names(flecs::entity e, std::string const &type, std::string const &rune_name, bool add, ModRuneData const &rune_data) {
	const int level = rune_data.level;
	const int flat_buff = rune_data.flat_buff;
	const int base = rune_data.base;
	const int upgrade = rune_data.upgrade;
	const int range = rune_data.range;
	const int duration_ticks = rune_data.duration_ticks;
	if (rune_name == "AffinityBuffRuneRegular") {
		mod_rune_type<AffinityBuffRuneRegular, godoctopus::BaseStats>(e, add, type, makeFlatBuff<AffinityBuffRuneRegular>(flat_buff));
	}
	else if (rune_name == "ArmorBuffRuneRegular") {
		mod_rune_type<ArmorBuffRuneRegular, godoctopus::BaseStats>(e, add, type, makeFlatBuff<ArmorBuffRuneRegular>(flat_buff));
	}
	else if (rune_name == "DamageBuffRuneRegular") {
		mod_rune_type<DamageBuffRuneRegular, godoctopus::BaseStats>(e, add, type, makeFlatBuff<DamageBuffRuneRegular>(flat_buff));
	}
	else if (rune_name == "HitPointBuffRuneRegular") {
		mod_rune_type<HitPointBuffRuneRegular, godoctopus::BaseStats>(e, add, type, makeFlatBuff<HitPointBuffRuneRegular>(flat_buff));
	}
	else if (rune_name == "ReloadBuffRuneRegular") {
		mod_rune_type<ReloadBuffRuneRegular, godoctopus::BaseStats>(e, add, type, makeFlatBuff<ReloadBuffRuneRegular>(flat_buff));
	}
	else if(rune_name == "AddRuneLoadOnAttack") {
		mod_rune_type_add_component(e, add, type, AddRuneLoadOnAttack{level});
	}
	else if (rune_name == "AddRuneLoadOnHit") {
		mod_rune_type_add_component(e, add, type, AddRuneLoadOnHit{level});
	}
	else if(rune_name == "AddRuneLoadOnTargetOnAttack") {
		mod_rune_type_add_component(e, add, type, AddRuneLoadOnTargetOnAttack{level});
	}
	else if (rune_name == "AoeDamageBasedOnDamageOnDeath") {
		mod_rune_type_add_component<AoeDamageBasedOnDamageOnDeath>(e, add, type, makeScalingRange<AoeDamageBasedOnDamageOnDeath>(base, upgrade, range));
	}
	else if (rune_name == "AoeDamageBasedOnHitpointOnDeath") {
		mod_rune_type_add_component<AoeDamageBasedOnHitpointOnDeath>(e, add, type, makeScalingRange<AoeDamageBasedOnHitpointOnDeath>(base, upgrade, range));
	}
	else if (rune_name == "AoeDamageConsumeRuneOnHit") {
		mod_rune_type_add_component<AoeDamageConsumeRuneOnHit>(e, add, type, makeScalingRange<AoeDamageConsumeRuneOnHit>(base, upgrade, range));
	}
	else if (rune_name == "AoeDamageSpecial") {
		mod_rune_type_add_component<AoeDamageSpecial>(e, add, type, makeScalingRange<AoeDamageSpecial>(base, upgrade, range));
	}
	else if (rune_name == "AoeHealBasedOnDamageOnDeath") {
		mod_rune_type_add_component<AoeHealBasedOnDamageOnDeath>(e, add, type, makeScalingRange<AoeHealBasedOnDamageOnDeath>(base, upgrade, range));
	}
	else if (rune_name == "AoeHealBasedOnHitpointOnDeath") {
		mod_rune_type_add_component<AoeHealBasedOnHitpointOnDeath>(e, add, type, makeScalingRange<AoeHealBasedOnHitpointOnDeath>(base, upgrade, range));
	}
	else if (rune_name == "ApplyArmorBuffAreaOnRuneLoad") {
		mod_rune_type_add_component<ApplyArmorBuffAreaOnRuneLoad>(e, add, type, makeScalingRangeDurationTicks<ApplyArmorBuffAreaOnRuneLoad>(base, upgrade, range, duration_ticks));
	}
	else if (rune_name == "ApplyAttackSpeedBuffOnRuneLoad") {
		mod_rune_type_add_component<ApplyAttackSpeedBuffOnRuneLoad>(e, add, type, makeScalingDurationTicks<ApplyAttackSpeedBuffOnRuneLoad>(base, upgrade, duration_ticks));
	}
	else if (rune_name == "ApplyAttackSpeedDebuffAreaOnRuneLoad") {
		mod_rune_type_add_component<ApplyAttackSpeedDebuffAreaOnRuneLoad>(e, add, type, makeScalingRangeDurationTicks<ApplyAttackSpeedDebuffAreaOnRuneLoad>(base, upgrade, range, duration_ticks));
	}
	else if (rune_name == "ApplyDamageBuffOnRuneLoad") {
		mod_rune_type_add_component<ApplyDamageBuffOnRuneLoad>(e, add, type, makeScalingDurationTicks<ApplyDamageBuffOnRuneLoad>(base, upgrade, duration_ticks));
	}
	else if (rune_name == "ApplyDamageDebuffAreaOnRuneLoad") {
		mod_rune_type_add_component<ApplyDamageDebuffAreaOnRuneLoad>(e, add, type, makeScalingRangeDurationTicks<ApplyDamageDebuffAreaOnRuneLoad>(base, upgrade, range, duration_ticks));
	}
	else if (rune_name == "ApplyUndyingBuffOnRuneLoad") {
		mod_rune_type_add_component(e, add, type, ApplyUndyingBuffOnRuneLoad{level});
	}
	else if (rune_name == "ArmorBuffRuneSpecial") {
		mod_rune_type<ArmorBuffRuneSpecial, godoctopus::BaseStats>(e, add, type, makeScaling<ArmorBuffRuneSpecial>(base, upgrade));
	}
	else if (rune_name == "ConditionalArmorBuffHighLifeRuneTier1") {
		mod_rune_type<ConditionalArmorBuffHighLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(e, add, type, makeScalingPercent<ConditionalArmorBuffHighLifeRuneTier1>(base, upgrade, 80));
	}
	else if (rune_name == "ConditionalArmorBuffLowLifeRuneTier1") {
		mod_rune_type<ConditionalArmorBuffLowLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(e, add, type, makeScalingPercent<ConditionalArmorBuffLowLifeRuneTier1>(base, upgrade, 80));
	}
	else if (rune_name == "ConditionalDamageBuffHighLifeRuneTier1") {
		mod_rune_type<ConditionalDamageBuffHighLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(e, add, type, makeScalingPercent<ConditionalDamageBuffHighLifeRuneTier1>(base, upgrade, 80));
	}
	else if (rune_name == "ConditionalDamageBuffLowLifeRuneTier1") {
		mod_rune_type<ConditionalDamageBuffLowLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(e, add, type, makeScalingPercent<ConditionalDamageBuffLowLifeRuneTier1>(base, upgrade, 80));
	}
	else if (rune_name == "ConditionalReloadBuffHighLifeRuneTier1") {
		mod_rune_type<ConditionalReloadBuffHighLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(e, add, type, makeScalingPercent<ConditionalReloadBuffHighLifeRuneTier1>(base, upgrade, 80));
	}
	else if (rune_name == "ConditionalReloadBuffLowLifeRuneTier1") {
		mod_rune_type<ConditionalReloadBuffLowLifeRuneTier1, godoctopus::BaseStats, octopus::HitPoint, octopus::HitPointMax>(e, add, type, makeScalingPercent<ConditionalReloadBuffLowLifeRuneTier1>(base, upgrade, 80));
	}
	else if (rune_name == "DamageBuffRuneSpecial") {
		std::cout<<"Adding Rune DamageBuffRuneSpecial with base: " << base << ", upgrade: " << upgrade << std::endl;
		mod_rune_type<DamageBuffRuneSpecial, godoctopus::BaseStats>(e, add, type, makeScaling<DamageBuffRuneSpecial>(base, upgrade));
	}
	else if (rune_name == "HitPointBuffRuneSpecial") {
		mod_rune_type<HitPointBuffRuneSpecial, godoctopus::BaseStats>(e, add, type, makeScaling<HitPointBuffRuneSpecial>(base, upgrade));
	}
	else if (rune_name == "ReloadBuffRuneSpecial") {
		mod_rune_type<ReloadBuffRuneSpecial, godoctopus::BaseStats>(e, add, type, makeScaling<ReloadBuffRuneSpecial>(base, upgrade));
	}
	else if (rune_name == "SpawnCloneUnitRune") {
		mod_rune_type_add_component(e, add, type, SpawnCloneUnitRune{level});
	}
	// Those runes are used in testing but not in game (yet)
	else if (rune_name == "ApplyArmorBuffOnRuneLoad") {
		mod_rune_type_add_component(e, add, type, makeScalingDurationTicks<ApplyArmorBuffOnRuneLoad>(base, upgrade, duration_ticks));
	}
	else if (rune_name == "ApplyDamageBuffAreaOnRuneLoad") {
		mod_rune_type_add_component(e, add, type, makeScalingRangeDurationTicks<ApplyDamageBuffAreaOnRuneLoad>(base, upgrade, range, duration_ticks));
	}
	// New Gen
	else if (rune_name == "RuneStats1") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats1>(base, rune_data));
	}
	else if (rune_name == "RuneStats2") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats2>(base, rune_data));
	}
	else if (rune_name == "RuneStats3") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats3>(base, rune_data));
	}
	else if (rune_name == "RuneStats4") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats4>(base, rune_data));
	}
	else if (rune_name == "RuneStats5") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats5>(base, rune_data));
	}
	else if (rune_name == "RuneStats6") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats6>(base, rune_data));
	}
	else if (rune_name == "RuneStats7") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats7>(base, rune_data));
	}
	else if (rune_name == "RuneStats8") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats8>(base, rune_data));
	}
	else if (rune_name == "RuneStats9") {
		mod_rune_type_add_component(e, add, type, makeStatsModifierRecorder<RuneStats9>(base, rune_data));
	}
	else if (rune_name == "AoePulseDamages") {
		mod_rune_type_add_component(e, add, type, make_aoe_pulse_rune<AoePulseDamages>(rune_data));
	}
	else if (rune_name == "LifestealRune") {
		mod_rune_type_add_component(e, add, type, make_lifesteal_rune(rune_data));
	}
	else if (rune_name == "DamageOnDeathRune") {
		mod_rune_type_add_component(e, add, type, make_aoe_on_death_rune<DamageOnDeathRune>(rune_data));
	}
	else if (rune_name == "HealOnDeathRune") {
		mod_rune_type_add_component(e, add, type, make_aoe_on_death_rune<HealOnDeathRune>(rune_data));
	}
	else {
		print_line("mod_rune_based_on_names: Unknown rune name ", rune_name.c_str());
	}
}
