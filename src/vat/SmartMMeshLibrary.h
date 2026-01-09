#pragma once

#include "scene/3d/node_3d.h"
#include "SmartMultiMeshInstance.h"

#include <mutex>

namespace godot {

class SmartMMeshLibrary : public Node3D {
	GDCLASS(SmartMMeshLibrary, Node3D)

	SET_GET_PARAM(TypedArray<Object>, smarts);
	/// @brief expected duration of a timestep
	SET_GET_PARAM_DEF(double, time_step, 0.01);
public:
	~SmartMMeshLibrary() {}

	SmartMultiMeshInstance* get_multi_mesh(int idx) const {
		return vec_multi_mesh[idx];
	}

	int size() const {
		return vec_multi_mesh.size();
	}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods()
	{
		ClassDB::bind_method(D_METHOD("set_smarts", "smarts"), &SmartMMeshLibrary::set_smarts);
		ClassDB::bind_method(D_METHOD("get_smarts"), &SmartMMeshLibrary::get_smarts);
		String arrayType = vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_NODE_TYPE, "SmartMultiMeshInstance");
		ClassDB::add_property("SmartMMeshLibrary", PropertyInfo(Variant::ARRAY, "smarts", PROPERTY_HINT_TYPE_STRING, arrayType, PROPERTY_HINT_ARRAY_TYPE),
			"set_smarts", "get_smarts");

		ADD_SIMPLE_PROP(SmartMMeshLibrary, FLOAT, time_step);

		ClassDB::bind_method(D_METHOD("get_multi_mesh", "idx"), &SmartMMeshLibrary::get_multi_mesh);
	}
	void _ready();

	void swap_transforms();

	// mutex used to lock during display to avoid syncing error while rendering
	std::mutex _mutex;
protected:
	void _notification(int p_notification);

	std::vector<SmartMultiMeshInstance*> vec_multi_mesh;
};

}
