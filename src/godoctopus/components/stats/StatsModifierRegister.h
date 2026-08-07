#pragma once

#include "StatsModifier.h"
#include "flecs.h"

namespace godoctopus {

template<typename T>
struct StatsModifierRegister {
	StatsModifier modifier;
	uint32_t index = 0;
	bool registered = false;
};

template<typename T>
void declare_modifier_register(flecs::world &ecs) {

	ecs.component<StatsModifierRegister<T>>()
		.member("index", &StatsModifierRegister<T>::index)
		.member("registered", &StatsModifierRegister<T>::registered)
		.member("modifier", &StatsModifierRegister<T>::modifier);

	ecs.observer<StatsModifierList, StatsModifierRegister<T>>()
		.event(flecs::OnSet)
		.each([](flecs::entity e, StatsModifierList &stats_modifier_list, StatsModifierRegister<T> &reg) {
			if (!reg.registered) {
				reg.index = add_modifier(stats_modifier_list, reg.modifier);
				reg.registered = true;
			}
		});

	ecs.observer<StatsModifierList, StatsModifierRegister<T>>()
		.event(flecs::OnRemove)
		.each([](flecs::entity e, StatsModifierList &stats_modifier_list, StatsModifierRegister<T> &reg) {
			if (reg.registered) {
				remove_modifier(stats_modifier_list, reg.index);
				reg.registered = false;
			}
		});
}

} // namespace godoctopus
