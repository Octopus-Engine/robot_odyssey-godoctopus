#pragma once

#include "core/math/color.h"

#include "godot_tools.h"

namespace godot {

struct VatInstanceData {

public:
	VatInstanceData() {}

	Color pickable_color;
	int track_number = 0;
	float track_offset = 0;
	float track_speed = 1.;
	double end_time = -1;
	bool alt_texture = false;
};

}
