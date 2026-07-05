#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class InfoProductionQueueResource : public Resource {
	GDCLASS(InfoProductionQueueResource, Resource)

	SET_GET_PARAM_DEF(String, prod_name, "");
	SET_GET_PARAM_DEF(double, progress, 0);
	SET_GET_PARAM_DEF(int, queue_idx, 0);
	SET_GET_PARAM_DEF(int64_t, source_entity_id, -1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoProductionQueueResource, STRING, prod_name);
		ADD_SIMPLE_PROP(InfoProductionQueueResource, FLOAT, progress);
		ADD_SIMPLE_PROP(InfoProductionQueueResource, INT, queue_idx);
		ADD_SIMPLE_PROP(InfoProductionQueueResource, INT, source_entity_id);
	}
};

}
