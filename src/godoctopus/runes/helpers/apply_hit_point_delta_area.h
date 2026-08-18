#pragma once

#include "flecs.h"
#include "octopus_types.h"

void apply_hit_point_delta_area(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx,
	octopus::Fixed range, octopus::Fixed delta);
