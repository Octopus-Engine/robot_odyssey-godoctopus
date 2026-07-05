#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"

namespace godot {

class InfoAvailableActionResource : public Resource {
	GDCLASS(InfoAvailableActionResource, Resource)
public:
	enum ActionType {
		ACTION_CAST = 0,
		ACTION_PRODUCTION = 1,
	};

private:
	SET_GET_PARAM_DEF(String, action_name, "");
	SET_GET_PARAM_DEF(int, action_type, ACTION_CAST);
	SET_GET_PARAM_DEF(int64_t, source_entity_id, -1);
public:
	static void _bind_methods() {
		ADD_SIMPLE_PROP(InfoAvailableActionResource, STRING, action_name);
		ADD_SIMPLE_PROP(InfoAvailableActionResource, INT, action_type);
		ADD_SIMPLE_PROP(InfoAvailableActionResource, INT, source_entity_id);
	}
};

}
