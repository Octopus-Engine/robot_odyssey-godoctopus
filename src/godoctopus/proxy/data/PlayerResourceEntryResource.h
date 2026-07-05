#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class PlayerResourceEntryResource : public Resource {
	GDCLASS(PlayerResourceEntryResource, Resource)

	SET_GET_PARAM_DEF(String, resource_name, "");
	SET_GET_PARAM_DEF(int64_t, amount, 0);
	SET_GET_PARAM_DEF(int64_t, cap, 0);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerResourceEntryResource, STRING, resource_name);
		ADD_SIMPLE_PROP(PlayerResourceEntryResource, INT, amount);
		ADD_SIMPLE_PROP(PlayerResourceEntryResource, INT, cap);
	}
};

}
