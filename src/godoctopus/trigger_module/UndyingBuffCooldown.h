#pragma once

#include "octopus_types.h"

// Cooldown component applied when TemporaryUndyingBuff_15s expires
// Prevents new buffs from being applied until this cooldown expires
// Duration is inversely scaled by special_value: (15*TICK_RATE) / (1 + special_value)
struct UndyingBuffCooldown
{
	int64_t cooldown_end_tick = 0;  // Tick when cooldown expires
};
