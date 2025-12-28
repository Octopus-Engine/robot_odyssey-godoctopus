#pragma once

#include "scene/3d/node_3d.h"
#include "VatMultiMeshInstance.h"

#include <mutex>

namespace godot {

class VatLibrary : public Node3D {
	GDCLASS(VatLibrary, Node3D)

	SET_GET_PARAM(TypedArray<Ref<VatAnimationTrack>>, tracks);
	/// @brief expected duration of a timestep
	SET_GET_PARAM_DEF(double, time_step, 0.01);
public:
	~VatLibrary() {}

	VatMultiMeshInstance* get_multi_mesh(int idx) const { return vec_multi_mesh[idx]; }

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods()
	{
		ADD_ARRAY_OBJECT_PROP(VatLibrary, VatAnimationTrack, tracks);
		ADD_SIMPLE_PROP(VatLibrary, FLOAT, time_step);

		ClassDB::bind_method(D_METHOD("get_multi_mesh", "idx"), &VatLibrary::get_multi_mesh);
	}
	void _ready();

	void swap_transforms();

	// mutex used to lock during display to avoid syncing error while rendering
	std::mutex _mutex;
protected:
	void _notification(int p_notification);

	std::vector<VatMultiMeshInstance*> vec_multi_mesh;
};

}
