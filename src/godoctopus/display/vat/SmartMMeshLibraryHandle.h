#pragma once

#include "flecs.h"

#include "vat/SmartMMeshLibrary.h"

#include "octopus/utils/FixedPoint.hh"

struct SmartMMeshLibraryHandle {
	int32_t multi_mesh_id = -1;
	octopus::Fixed r = 1.;
	octopus::Fixed g = 1.;
	octopus::Fixed b = 1.;
	octopus::Fixed a = 1.;
	octopus::Fixed scale = 1.;
	octopus::Fixed up = 1.;
	int32_t instance_id = -1;
};

struct SmartMMeshProperties {
};

void declare_smart_mmesh_library_systems(flecs::world &ecs, godot::SmartMMeshLibrary *library);
