#pragma once

#include "core/io/resource.h"

#include "godoctopus/proxy/data/PlayerLoadoutRuneSlotResource.h"
#include "godot_tools.h"

namespace godot {

class PlayerLoadoutUnitEntryResource : public Resource {
	GDCLASS(PlayerLoadoutUnitEntryResource, Resource)

	SET_GET_PARAM_DEF(String, prefab_name, "");
	SET_GET_PARAM(TypedArray<Ref<PlayerLoadoutRuneSlotResource>>, slots);

public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerLoadoutUnitEntryResource, STRING, prefab_name);
		ADD_ARRAY_OBJECT_PROP(PlayerLoadoutUnitEntryResource, PlayerLoadoutRuneSlotResource, slots);
	}
};

}
