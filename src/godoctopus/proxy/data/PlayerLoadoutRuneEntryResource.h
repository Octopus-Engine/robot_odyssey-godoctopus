#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class PlayerLoadoutRuneEntryResource : public Resource {
	GDCLASS(PlayerLoadoutRuneEntryResource, Resource)

	SET_GET_PARAM_DEF(String, rune_internal_name, "");
	SET_GET_PARAM_DEF(String, rune_resource_path, "");
	SET_GET_PARAM_DEF(int64_t, rune_level, 1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerLoadoutRuneEntryResource, STRING, rune_internal_name);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneEntryResource, STRING, rune_resource_path);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneEntryResource, INT, rune_level);
	}
};

}
