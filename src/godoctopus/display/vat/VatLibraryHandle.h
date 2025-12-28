#pragma once

#include "flecs.h"

#include "vat/VatLibrary.h"

struct VatLibraryHandle {
	int32_t multi_mesh_id = -1;
	int32_t instance_id = -1;
};

void declare_vat_library_systems(flecs::world &ecs, godot::VatLibrary *library);
