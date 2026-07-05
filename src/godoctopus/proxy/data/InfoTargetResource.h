#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class InfoTargetResource : public Resource {
	GDCLASS(InfoTargetResource, Resource)

	SET_GET_PARAM_DEF(Vector2, position, Vector2());
	SET_GET_PARAM_DEF(bool, is_attack, false);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoTargetResource, VECTOR2, position);
		ADD_SIMPLE_PROP(InfoTargetResource, BOOL, is_attack);
	}
};

}
