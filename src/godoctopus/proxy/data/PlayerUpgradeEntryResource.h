#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class PlayerUpgradeEntryResource : public Resource {
	GDCLASS(PlayerUpgradeEntryResource, Resource)

	SET_GET_PARAM_DEF(String, upgrade_name, "");
	SET_GET_PARAM_DEF(int64_t, level, 0);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(PlayerUpgradeEntryResource, STRING, upgrade_name);
		ADD_SIMPLE_PROP(PlayerUpgradeEntryResource, INT, level);
	}
};

}
