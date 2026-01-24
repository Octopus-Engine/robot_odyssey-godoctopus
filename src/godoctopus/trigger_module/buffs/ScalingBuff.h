#pragma once

#include "godoctopus/components/special/Special.h"

template<typename BuffType, typename... Components>
struct LeveledBuff : public BuffType {
	void apply(flecs::entity e, Components... comps) const {
		BuffType::apply(e, level, comps...);
	}
	void revert(flecs::entity e, Components... comps) const {
		BuffType::revert(e, level, comps...);
	}

	int32_t level = 0;
};

template<typename BuffType, typename... Components>
struct SpecialScaledBuff : public BuffType {
	void apply(flecs::entity e, Components... comps) const {
		BuffType::apply(e, get_special_value(e), comps...);
	}
	void revert(flecs::entity e, Components... comps) const {
		BuffType::revert(e, get_special_value(e), comps...);
	}
};
