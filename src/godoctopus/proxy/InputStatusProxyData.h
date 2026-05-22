#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class InputStatusResourceCost : public Resource {
	GDCLASS(InputStatusResourceCost, Resource)

	SET_GET_PARAM_DEF(String, resource, "");
	SET_GET_PARAM_DEF(double, amount, 0.0);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InputStatusResourceCost, STRING, resource);
		ADD_SIMPLE_PROP(InputStatusResourceCost, FLOAT, amount);
	}
};

class InputStatusResource : public Resource {
	GDCLASS(InputStatusResource, Resource)

	SET_GET_PARAM_DEF(bool, ok, true);
	SET_GET_PARAM_DEF(int64_t, entity_id, 0);
	SET_GET_PARAM(TypedArray<Ref<InputStatusResourceCost>>, resource_cost);
	SET_GET_PARAM(TypedArray<String>, missing_upgrades);
	SET_GET_PARAM(TypedArray<String>, other_explanations);
	SET_GET_PARAM_DEF(double, cooldown_ratio, 0.0);
	SET_GET_PARAM_DEF(int64_t, cooldown_ticks_remaining, 0);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InputStatusResource, BOOL, ok);
		ADD_SIMPLE_PROP(InputStatusResource, INT, entity_id);
		ADD_ARRAY_OBJECT_PROP(InputStatusResource, InputStatusResourceCost, resource_cost);
		ADD_ARRAY_PROP(InputStatusResource, missing_upgrades);
		ADD_ARRAY_PROP(InputStatusResource, other_explanations);
		ADD_SIMPLE_PROP(InputStatusResource, FLOAT, cooldown_ratio);
		ADD_SIMPLE_PROP(InputStatusResource, INT, cooldown_ticks_remaining);
	}
};

} // namespace godot
