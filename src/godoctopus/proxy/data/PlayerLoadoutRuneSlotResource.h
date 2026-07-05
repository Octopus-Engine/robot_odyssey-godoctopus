#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class PlayerLoadoutRuneSlotResource : public Resource {
	GDCLASS(PlayerLoadoutRuneSlotResource, Resource)

	SET_GET_PARAM_DEF(int32_t, slot_type, 0);
	SET_GET_PARAM_DEF(bool, activated, true);
	SET_GET_PARAM_DEF(bool, locked, false);
	SET_GET_PARAM_DEF(bool, has_rune, false);
	SET_GET_PARAM_DEF(String, rune_internal_name, "");
	SET_GET_PARAM_DEF(String, rune_resource_path, "");
	SET_GET_PARAM_DEF(int64_t, rune_level, 1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, INT, slot_type);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, BOOL, activated);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, BOOL, locked);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, BOOL, has_rune);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, STRING, rune_internal_name);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, STRING, rune_resource_path);
		ADD_SIMPLE_PROP(PlayerLoadoutRuneSlotResource, INT, rune_level);
	}
};

}
