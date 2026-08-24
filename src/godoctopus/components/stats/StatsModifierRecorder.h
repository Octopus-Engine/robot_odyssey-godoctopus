#pragma once

#include "StatsModifierList.h"
#include "StatsModifierRegister.h"
#include "flecs.h"

namespace godoctopus {

template<typename T>
struct StatsModifierRecorder {
	StatsModifier modifier;
	uint32_t list_idx = 0;
};

template<typename T>
struct StatsModifierRecord {
	uint32_t list_idx = 0;
	uint32_t index = 0;
	bool registered = false;
};

template<typename T>
void add_stats_modifier_record(flecs::entity e, StatsModifierRegister &stats_modifier_register, const StatsModifierRecorder<T> &recorder) {
	StatsModifierRecord<T> record;
	record.index = add_modifier(stats_modifier_register.lists[recorder.list_idx], recorder.modifier);
	record.list_idx = recorder.list_idx;
	record.registered = true;
	e.set<StatsModifierRecord<T>>(record);
}

template<typename T>
void remove_stats_modifier_record(flecs::entity e, StatsModifierRegister &stats_modifier_register) {
	StatsModifierRecord<T> *old_record = e.try_get_mut<StatsModifierRecord<T>>();
	if (old_record) {
		if (old_record->registered) {
			remove_modifier(stats_modifier_register.lists[old_record->list_idx], old_record->index);
			old_record->registered = false;
		}
		e.remove<StatsModifierRecord<T>>();
	}
}

template<typename T>
void declare_modifier_register(flecs::world &ecs) {

	ecs.component<StatsModifierRecorder<T>>()
		.member("modifier", &StatsModifierRecorder<T>::modifier)
		.member("list_idx", &StatsModifierRecorder<T>::list_idx)
	;
	ecs.component<StatsModifierRecord<T>>()
		.member("index", &StatsModifierRecord<T>::index)
		.member("registered", &StatsModifierRecord<T>::registered)
	;

	ecs.observer<StatsModifierRegister, StatsModifierRecorder<T>>()
		.event(flecs::OnSet)
		.template write<StatsModifierRecord<T>>()
		.each([](flecs::entity e, StatsModifierRegister &stats_modifier_register, StatsModifierRecorder<T> &rec) {
			remove_stats_modifier_record<T>(e, stats_modifier_register);
			add_stats_modifier_record<T>(e, stats_modifier_register, rec);
		});

	ecs.observer<StatsModifierRegister, StatsModifierRecorder<T>>()
		.event(flecs::OnRemove)
		.template write<StatsModifierRecord<T>>()
		.each([](flecs::entity e, StatsModifierRegister &stats_modifier_register, StatsModifierRecorder<T> &rec) {
			remove_stats_modifier_record<T>(e, stats_modifier_register);
		});
}

} // namespace godoctopus
