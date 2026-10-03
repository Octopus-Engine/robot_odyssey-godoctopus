#pragma once

#include "scene/3d/node_3d.h"
#include "scene/resources/curve.h"
#include "vat/SmartMultiMeshInstance.h"

#include "smart_list/smart_list.h"
#include "godot_tools.h"
#include "core/math/random_number_generator.h"

namespace godot {

struct RayData {
	Color color;
	Transform3D transform;
	Vector3 scale = Vector3(1,1,1);
	int resource = -1;
	double lifetime = 0.;
	double size = 0.;
};

class RaySmartResource : public Resource {
	GDCLASS(RaySmartResource, Resource)

	SET_GET_PARAM_DEF(double, time, 1.0);
	SET_GET_PARAM_DEF(bool, size_absolute, true);
	SET_GET_PARAM(Ref<Curve>, progress);
	SET_GET_PARAM(Ref<Curve>, size);
	SET_GET_PARAM(Ref<Curve>, width);
	SET_GET_PARAM(Ref<Curve>, instance_data);
public:
	RaySmartResource() {}
	~RaySmartResource() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_SIMPLE_PROP(RaySmartResource, FLOAT, time);
		ADD_SIMPLE_PROP(RaySmartResource, BOOL, size_absolute);
		ADD_OBJECT_PROP(RaySmartResource, Curve, progress);
		ADD_OBJECT_PROP(RaySmartResource, Curve, size);
		ADD_OBJECT_PROP(RaySmartResource, Curve, width);
		ADD_OBJECT_PROP(RaySmartResource, Curve, instance_data);
	}
};

class RaySmartMMesh : public MultiMeshInstance3D {
	GDCLASS(RaySmartMMesh, MultiMeshInstance3D)

	SET_GET_PARAM(String, key);
	SET_GET_PARAM(Ref<RaySmartResource>, default_resource);
	SET_GET_PARAM(TypedArray<RaySmartResource>, ray_resources);

public:
	~RaySmartMMesh() {}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_SIMPLE_PROP(RaySmartMMesh, STRING, key);
		ADD_OBJECT_PROP(RaySmartMMesh, RaySmartResource, default_resource);
		ADD_ARRAY_OBJECT_PROP(RaySmartMMesh, RaySmartResource, ray_resources);

		ClassDB::bind_method(D_METHOD("add_instance", "pos", "target", "color"), &RaySmartMMesh::add_instance);
		ClassDB::bind_method(D_METHOD("add_instance_custom", "pos", "target", "color", "resource"), &RaySmartMMesh::add_instance_custom);
	}
	void _ready();
	void _process(double delta);

	void add_instance(Vector3 const &pos, Vector3 const &target, Color const &color);
	void add_instance_custom(Vector3 const &pos, Vector3 const &target, Color const &color, int resource);
protected:
	std::mutex _mutex;
	void _notification(int p_notification);

	smart_list<RayData> data;
	double elapsed = 0.;
	Ref<RandomNumberGenerator> rng;
};

}
