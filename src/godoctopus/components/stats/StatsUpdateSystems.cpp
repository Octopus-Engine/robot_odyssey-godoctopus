#include "StatsUpdateSystems.h"

#include "flecs.h"
#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/serialization/utils/UtilsSupport.hh"
#include "octopus/serialization/containers/VectorSupport.hh"
#include "StatsSet.h"
#include "StatsModifierRegister.h"

#include "octopus_types.h"

namespace godoctopus {

void declare_stateupdate_systems(flecs::world &ecs) {
	ecs.component<std::vector<octopus::Fixed>>()
        .opaque(std_vector_support<octopus::Fixed>);

	ecs.component<std::vector<uint32_t>>()
        .opaque(std_vector_support<uint32_t>);

	ecs.component<StatsSet>()
		.member("values", &StatsSet::values);
	ecs.component<BaseStats>()
		.member("stats", &BaseStats::stats);
	ecs.component<CurrentStats>()
		.member("stats", &CurrentStats::stats);

	ecs.component<StatsModifier>()
		.member("type", &StatsModifier::type)
		.member("base_delta", &StatsModifier::base_delta)
		.member("coefficients", &StatsModifier::coefficients);

	ecs.component<StatsModifierList::Entry>()
		.member("modifier", &StatsModifierList::Entry::modifier)
		.member("stack_count", &StatsModifierList::Entry::stack_count)
		.member("slot_index", &StatsModifierList::Entry::slot_index);

    ecs.component<std::vector<StatsModifierList::Entry>>()
        .opaque(std_vector_support<StatsModifierList::Entry>);

	ecs.component<StatsModifierList>()
		.member("entries", &StatsModifierList::entries)
		.member("slots", &StatsModifierList::slots)
		.member("free_head", &StatsModifierList::free_head);

	ecs.component<StatsModifierRegister>()
		.member("lists", &StatsModifierRegister::lists);

	using namespace octopus;
	// reset stats to base stats at the beginning of each update
	ecs.system<BaseStats const, CurrentStats>()
		.kind(ecs.entity(ValidatePhase))
		.multi_threaded()
		.each([](flecs::entity e, BaseStats const &base, CurrentStats &current) {
			current.stats = base.stats;
		});

	// apply all modifiers
	ecs.system<StatsModifierRegister, CurrentStats>()
		.kind(ecs.entity(ValidatePhase))
		.multi_threaded()
		.each([](flecs::entity e, StatsModifierRegister &reg, CurrentStats &current) {
			compact(reg.lists[0]);
			apply_modifiers(current.stats, reg.lists[0]);
			compact(reg.lists[1]);
			apply_modifiers(current.stats, reg.lists[1]);
			compact(reg.lists[2]);
			apply_modifiers(current.stats, reg.lists[2]);
		});

	// update engine stats base on current stats
	ecs.system<CurrentStats const, Attack>()
		.kind(ecs.entity(ValidatePhase))
		.multi_threaded()
		.each([](flecs::entity e, CurrentStats const &current, Attack &attack) {
			attack.cst.damage = current.stats.values[Damage];
			attack.cst.reload_time = static_cast<int32_t>(((100*TICK_RATE)/current.stats.values[Speed]).to_int());
		});

	ecs.system<CurrentStats const, HitPoint, HitPointMax>()
		.kind(ecs.entity(ValidatePhase))
		.multi_threaded()
		.each([](flecs::entity e, CurrentStats const &current, HitPoint &hp, HitPointMax &hp_max) {
			Fixed delta = current.stats.values[HitPoints] - hp_max.qty;
			if (delta > 0) {
				hp.qty += delta;
			}
			hp_max.qty = current.stats.values[HitPoints];
			if (hp.qty > hp_max.qty) {
				hp.qty = hp_max.qty;
			}
		});

	ecs.system<CurrentStats const, Armor>()
		.kind(ecs.entity(ValidatePhase))
		.multi_threaded()
		.each([](flecs::entity e, CurrentStats const &current, Armor &armor) {
			armor.qty = current.stats.values[Shield];
		});
}

} // namespace godoctopus
