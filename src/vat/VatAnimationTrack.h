#pragma once

#include "core/io/resource.h"
#include "scene/resources/mesh.h"
#include "scene/resources/material.h"

#include "godot_tools.h"

namespace godot {

class VatAnimationTrack : public Resource {
	GDCLASS(VatAnimationTrack, Resource)

public:
	VatAnimationTrack() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods()
	{
		ADD_OBJECT_PROP(VatAnimationTrack, Mesh, mesh);
		ADD_OBJECT_PROP(VatAnimationTrack, ShaderMaterial, material);
		ADD_SIMPLE_PROP(VatAnimationTrack, ARRAY, tracks);
		ADD_SIMPLE_PROP(VatAnimationTrack, VECTOR3, forward);
		ADD_SIMPLE_PROP(VatAnimationTrack, INT, walking_anim);
		ADD_SIMPLE_PROP(VatAnimationTrack, INT, idle_anim);
		ADD_SIMPLE_PROP(VatAnimationTrack, FLOAT, base_move_speed);
	}

	SET_GET_PARAM(Ref<Mesh>, mesh);
	SET_GET_PARAM(Ref<ShaderMaterial>, material);
	SET_GET_PARAM(TypedArray<Vector2i>, tracks);
	SET_GET_PARAM(Vector3, forward);
	SET_GET_PARAM_DEF(int, walking_anim, 0);
	SET_GET_PARAM_DEF(int, idle_anim, 0);
	SET_GET_PARAM_DEF(float, base_move_speed, 3.0);
};

}
