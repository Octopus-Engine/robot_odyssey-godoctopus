#pragma once

#include "StatsModifierList.h"
#include "StatsModifierRegister.h"
#include "flecs.h"

namespace godoctopus {

template<typename T>
struct StatsModifierRecorder {
	StatsModifier modifier;
	uint32_t list_idx = 0;
	uint32_t index = 0;
	bool registered = false;
};

template<typename T>
void declare_modifier_register(flecs::world &ecs) {

	ecs.component<StatsModifierRecorder<T>>()
		.member("modifier", &StatsModifierRecorder<T>::modifier)
		.member("list_idx", &StatsModifierRecorder<T>::list_idx)
		.member("index", &StatsModifierRecorder<T>::index)
		.member("registered", &StatsModifierRecorder<T>::registered)
	;

	ecs.observer<StatsModifierRegister, StatsModifierRecorder<T>>()
		.event(flecs::OnSet)
		.each([](flecs::entity e, StatsModifierRegister &stats_modifier_register, StatsModifierRecorder<T> &rec) {
			if (!rec.registered) {
				rec.index = add_modifier(stats_modifier_register.lists[rec.list_idx], rec.modifier);
				rec.registered = true;
			}
		});

	ecs.observer<StatsModifierRegister, StatsModifierRecorder<T>>()
		.event(flecs::OnRemove)
		.each([](flecs::entity e, StatsModifierRegister &stats_modifier_register, StatsModifierRecorder<T> &rec) {
			if (rec.registered) {
				remove_modifier(stats_modifier_register.lists[rec.list_idx], rec.index);
				rec.registered = false;
			}
		});
}

} // namespace godoctopus
