#pragma once

#include "flecs.h"

#include "vat/SmartMMeshLibrary.h"

#include "octopus/utils/FixedPoint.hh"
#include "godoctopus/display/vat/VatLibraryHandle.h"

template<typename T>
struct SmartMMeshLibraryHandleT {
	int32_t multi_mesh_id = -1;
	int32_t instance_id = -1;
};

struct DefaultHandle {};
using SmartMMeshLibraryHandle = SmartMMeshLibraryHandleT<DefaultHandle>;

struct ProjectileSmartMMesh {
	octopus::Fixed r = 1.;
	octopus::Fixed g = 1.;
	octopus::Fixed b = 1.;
	octopus::Fixed a = 1.;
	octopus::Fixed scale = 1.;
	octopus::Fixed up = 1.;
	octopus::Fixed end_up = octopus::Fixed::One();
	int64_t timestamp_start = 0;
	int64_t timestamp_end = 0;
};

struct RelativePosition {
	flecs::entity ref_entity;
	float x = 0.;
	float y = 0.;
	float z = 0.;
};

struct Selected {
	bool selected = false;
};

struct Vision {
	bool visible = true;
};

void declare_smart_mmesh_library_systems(flecs::world &ecs, godot::SmartMMeshLibrary *library, godot::VatLibrary *vat_library, int32_t selection_multi_mesh_id);
