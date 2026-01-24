#pragma once

#include "godoctopus/components/special/Special.h"

template<typename BuffType, typename... Components>
struct LeveledBuff : public BuffType {
	void apply(flecs::entity e, Components&... comps) const {
		BuffType::apply(e, level, comps...);
	}
	void revert(flecs::entity e, Components&... comps) const {
		BuffType::revert(e, level, comps...);
	}

	int32_t level = 0;
};

template<typename BuffType, typename... Components>
struct SpecialScaledBuff : public BuffType {
	void apply(flecs::entity e, Components&... comps) const {}
	void revert(flecs::entity e, Components&... comps) const {
		BuffType::revert(e, old_special, comps...);
	}

	void update_value(flecs::entity e, Components&... comps) {
		if (init) BuffType::revert(e, old_special, comps...);
		old_special = get_special_value(e);
		init = true;
		BuffType::apply(e, old_special, comps...);
	}

	// used to update properly when special changes
	octopus::Fixed old_special = 0;
	bool init = false;
};
