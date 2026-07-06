#pragma once

#include "godoctopus/trigger_module/TriggerDeclaration.h"

inline Dictionary create_rune_data(int level = 1, int flat_buff = 0, int base = 0, int level_upgrade = 0, int upgrade = 0, int range = 0, int duration_ticks = 0) {
	Dictionary rune_data;
	rune_data["level"] = level;
	rune_data["flat_buff"] = flat_buff;
	rune_data["base"] = base;
	rune_data["level_upgrade"] = level_upgrade;
	rune_data["upgrade"] = upgrade;
	rune_data["range"] = range;
	rune_data["duration_ticks"] = duration_ticks;
	return rune_data;
}
