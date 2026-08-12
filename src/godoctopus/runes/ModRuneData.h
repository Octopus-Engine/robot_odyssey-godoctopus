#pragma once

#include "godoctopus/components/stats/StatsSet.h"

struct ModRuneData {
	int level = 0;
	int flat_buff = 0;
	int base = 0;
	int level_upgrade = 0;
	int upgrade = 0;
	int range = 0;
	int duration_ticks = 0;
	godoctopus::StatsSet stats_set_coef;
	godoctopus::StatsType stats_type = godoctopus::StatsType::Affinity;
	int modifier_priority = 0;
};
