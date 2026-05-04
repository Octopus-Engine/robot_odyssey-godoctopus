#pragma once

#include "octopus/components/basic/hitpoint/HitPoint.hh"

// Temporary Undying Buff component - prevents HP from falling to 0 when active
template<int64_t duration_ticks>
struct TemporaryUndyingBuff{
	static constexpr int64_t DURATION_TICKS = duration_ticks;
	bool decoy = false;

	void apply(octopus::HitPoint &hp) const {
		// No stat modification needed - protection is enforced in ValidatePhase
	}

	void revert(octopus::HitPoint &hp) const {
		// No stat modification needed - protection removal is automatic when buff expires
	}
};

