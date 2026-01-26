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

template<typename BuffType>
struct SpecialUpdate {
	octopus::Fixed old_special = 0;
	bool init = false;
};

template<typename RuneType, typename BuffType, typename... Components>
struct SpecialScaledBuff : public BuffType {
	void apply(flecs::entity e, Components&... comps) const {}
	void revert(flecs::entity e, Components&... comps) const {
		auto &spec_up = e.get_mut<SpecialUpdate<RuneType>>();
		if (spec_up.init) BuffType::revert(e, spec_up.old_special, comps...);
		spec_up.init = false;
	}

	void update_value(flecs::entity e, SpecialUpdate<RuneType> &special_update, Components&... comps) {
		if (special_update.init) BuffType::revert(e, special_update.old_special, comps...);
		special_update.old_special = get_special_value(e);
		special_update.init = true;
		BuffType::apply(e, special_update.old_special, comps...);
	}
};
