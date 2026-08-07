#include "StatsModifierList.h"

namespace godoctopus {

uint32_t add_modifier(StatsModifierList &list, StatsModifier const &modifier) {
	uint32_t slot_idx = 0;
	if (list.free_head != StatsModifierList::INVALID_INDEX) {
		slot_idx = list.free_head;
		list.free_head = list.slots[slot_idx]; // next free index
	} else {
		slot_idx = list.slots.size();
		list.slots.push_back(StatsModifierList::INVALID_INDEX);
	}

	list.slots[slot_idx] = list.entries.size();
	list.entries.push_back({modifier, 1, slot_idx});

	return slot_idx;
}

void remove_modifier(StatsModifierList &list, uint32_t index) {
	if (index < list.entries.size()) {
		list.entries[list.slots[index]].stack_count = 0;
		list.slots[index] = list.free_head;
		list.free_head = index;
	}
}

void stack_modifier(StatsModifierList &list, uint32_t index) {
	if (index < list.slots.size()) {
		++list.entries[list.slots[index]].stack_count;
	}
}

void unstack_modifier(StatsModifierList &list, uint32_t index) {
	if (index < list.slots.size()) {
		const uint32_t entry_index = list.slots[index];
		if (entry_index < list.entries.size() && list.entries[entry_index].stack_count > 0) {
			--list.entries[entry_index].stack_count;
			if (list.entries[entry_index].stack_count == 0) {
				list.slots[index] = list.free_head;
				list.free_head = index;
			}
		}
	}
}

void apply_modifiers(StatsSet &stats, StatsModifierList const &list) {
	for (uint32_t i = 0; i < list.entries.size(); ++i) {
		if (list.entries[i].stack_count > 0) {
			apply_modifier(stats, list.entries[i].modifier, list.entries[i].stack_count);
		}
	}
}

void compact(StatsModifierList &list) {
	uint32_t write_idx = 0;
	for (uint32_t read_idx = 0; read_idx < list.entries.size(); ++read_idx) {
		if (list.entries[read_idx].stack_count > 0) {
			if (write_idx != read_idx) {
				list.entries[write_idx] = std::move(list.entries[read_idx]);
				list.slots[list.entries[write_idx].slot_index] = write_idx;
			}
			++write_idx;
		}
	}
	list.entries.resize(write_idx);
}

} // namespace godoctopus
