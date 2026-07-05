#pragma once

#include "godoctopus/components/special/Special.h"

template<typename BuffType, typename... Components>
struct FlatBuff : public BuffType {
	void apply(flecs::entity e, Components&... comps) const {
		BuffType::apply(e, qty, comps...);
	}
	void revert(flecs::entity e, Components&... comps) const {
		BuffType::revert(e, qty, comps...);
	}

	int64_t qty = 0;
};

template<typename BuffType>
struct SpecialUpdate {
	octopus::Fixed old_special = 0;
	bool active = false;
};

template<typename RuneType, typename BuffType, typename... Components>
struct SpecialScaledBuff : public BuffType {
	void apply(flecs::entity e, Components&... comps) const {}
	void revert(flecs::entity e, Components&... comps) const {
		auto &spec_up = e.get_mut<SpecialUpdate<RuneType>>();
		if (spec_up.active) BuffType::revert(e, get_bonus(spec_up.old_special), comps...);
		spec_up.active = false;
	}

	void update_value(flecs::entity e, SpecialUpdate<RuneType> &special_update, Components&... comps) {
		if (special_update.active) BuffType::revert(e, get_bonus(special_update.old_special), comps...);
		special_update.old_special = get_special_value(e);
		special_update.active = is_active(e, comps...);
		if (special_update.active) {
			BuffType::apply(e, get_bonus(special_update.old_special), comps...);
		}
	}

	int64_t base = 0;
	int64_t upgrade = 0;

	octopus::Fixed get_bonus(octopus::Fixed const &special) const {
		return base + (upgrade * special);
	}

	virtual ~SpecialScaledBuff() = default;
	virtual bool is_active(flecs::entity e, Components&... comps) const { return true; }
};
