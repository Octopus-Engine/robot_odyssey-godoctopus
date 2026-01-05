#pragma once

#include "scene/3d/node_3d.h"
#include "vat/SmartMultiMeshInstance.h"

#include "smart_list/smart_list.h"
#include "godot_tools.h"
#include "core/math/random_number_generator.h"

namespace godot {

struct ParticuleData {
	Color color;
	Vector3 scale = Vector3(1,1,1);
	int resource = -1;
	std::vector<Vector3> position;
	std::vector<Vector3> direction;
	std::vector<double> time_offset;
};

class ParticuleSmartResource : public Resource {
	GDCLASS(ParticuleSmartResource, Resource)

	SET_GET_PARAM(Ref<Curve>, scale_curve);
	SET_GET_PARAM(Ref<Curve>, speed_curve);
	SET_GET_PARAM_DEF(double, time, 1.0);
	SET_GET_PARAM_DEF(double, scatter, 1.0);
	SET_GET_PARAM_DEF(int, count, 8);
	SET_GET_PARAM_DEF(double, time_spread, 1.0);
public:
	ParticuleSmartResource() {}
	~ParticuleSmartResource() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_OBJECT_PROP(ParticuleSmartResource, Curve, scale_curve);
		ADD_OBJECT_PROP(ParticuleSmartResource, Curve, speed_curve);
		ADD_SIMPLE_PROP(ParticuleSmartResource, FLOAT, time);
		ADD_SIMPLE_PROP(ParticuleSmartResource, FLOAT, scatter);
		ADD_SIMPLE_PROP(ParticuleSmartResource, INT, count);
		ADD_SIMPLE_PROP(ParticuleSmartResource, FLOAT, time_spread);
	}
};

class ParticuleSmartMMesh : public MultiMeshInstance3D {
	GDCLASS(ParticuleSmartMMesh, MultiMeshInstance3D)

	SET_GET_PARAM(Ref<ParticuleSmartResource>, default_resource);
	SET_GET_PARAM(TypedArray<Ref<ParticuleSmartResource>>, particule_resources);

public:
	~ParticuleSmartMMesh() {}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_OBJECT_PROP(ParticuleSmartMMesh, ParticuleSmartResource, default_resource);
		ADD_ARRAY_OBJECT_PROP(ParticuleSmartMMesh, ParticuleSmartResource, particule_resources);

		ClassDB::bind_method(D_METHOD("add_instance", "pos", "color"), &ParticuleSmartMMesh::add_instance);
		ClassDB::bind_method(D_METHOD("add_instance_custom", "pos", "color", "resource"), &ParticuleSmartMMesh::add_instance_custom);
		ClassDB::bind_method(D_METHOD("add_instance_coned", "pos", "color", "count", "scale", "direction", "angle_spread", "resource"), &ParticuleSmartMMesh::add_instance_coned);
	}
	void _ready();
	void _process(double delta);

	void add_instance(Vector3 const &pos, Color const &color);
	void add_instance_custom(Vector3 const &pos, Color const &color, int resource);
	void add_instance_detailed(Vector3 const &pos, Color const &color, int c, Vector3 const &scale, int resource);
	void add_instance_coned(Vector3 const &pos, Color const &color, int c, Vector3 const &scale, Vector3 const &direction, float angle_spread, int resource);

protected:
	std::mutex _mutex;
	void _notification(int p_notification);

	smart_list<ParticuleData> data;
	double elapsed = 0.;
	Ref<RandomNumberGenerator> rng;
};

}
