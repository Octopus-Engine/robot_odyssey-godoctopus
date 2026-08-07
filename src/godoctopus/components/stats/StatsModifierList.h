#pragma once

#include <vector>
#include <limits>

#include "StatsModifier.h"

namespace godoctopus {

struct StatsModifierList {
	static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();
	struct Entry {
		StatsModifier modifier;
		uint32_t stack_count = 0;
		uint32_t slot_index = StatsModifierList::INVALID_INDEX;
	};

	std::vector<Entry> entries;
	std::vector<uint32_t> slots;
	uint32_t free_head = INVALID_INDEX;
}; // struct StatsModifierList

uint32_t add_modifier(StatsModifierList &list, StatsModifier const &modifier);
void remove_modifier(StatsModifierList &list, uint32_t index);
void stack_modifier(StatsModifierList &list, uint32_t index);
void unstack_modifier(StatsModifierList &list, uint32_t index);
void apply_modifiers(StatsSet &stats, StatsModifierList const &list);
void compact(StatsModifierList &list);

} // namespace godoctopus
