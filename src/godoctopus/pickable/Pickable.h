#pragma once

#include "scene/3d/node_3d.h"
#include "scene/main/viewport.h"

#include "vat/VatLibrary.h"
#include "godoctopus/pickable/PickerNode.h"

#include "flecs.h"
#include "octopus_types.h"

struct Pickable { bool decoy=false; };
struct PickableSetUp { int id; };

void declare_pickable_systems(flecs::world &ecs, godot::VatLibrary *library, godot::PickerNode *picker);
