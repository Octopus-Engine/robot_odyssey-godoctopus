#pragma once

#include "scene/3d/node_3d.h"
#include "vat/SmartMultiMeshInstance.h"

#include "smart_list/smart_list.h"
#include "godot_tools.h"
#include "core/math/random_number_generator.h"

namespace godot {

struct ParticuleData {
	std::vector<Vector3> position;
	std::vector<Vector3> direction;
	std::vector<double> time_offset;
};

class ParticuleSmartMMesh : public MultiMeshInstance3D {
	GDCLASS(ParticuleSmartMMesh, MultiMeshInstance3D)

	SET_GET_PARAM(Ref<Curve>, scale_curve);
	SET_GET_PARAM(Ref<Curve>, speed_curve);
	SET_GET_PARAM(double, time);
	SET_GET_PARAM(double, scatter);
	SET_GET_PARAM(int, count);

public:
	~ParticuleSmartMMesh() {}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_OBJECT_PROP(ParticuleSmartMMesh, Curve, scale_curve);
		ADD_OBJECT_PROP(ParticuleSmartMMesh, Curve, speed_curve);
		ADD_SIMPLE_PROP(ParticuleSmartMMesh, FLOAT, time);
		ADD_SIMPLE_PROP(ParticuleSmartMMesh, FLOAT, scatter);
		ADD_SIMPLE_PROP(ParticuleSmartMMesh, INT, count);

		ClassDB::bind_method(D_METHOD("add_instance", "pos"), &ParticuleSmartMMesh::add_instance);
	}
	void _ready();
	void _process(double delta);

	void add_instance(Vector3 const &pos);

protected:
	std::mutex _mutex;
	void _notification(int p_notification);

	smart_list<ParticuleData> data;
	double elapsed = 0.;
	Ref<RandomNumberGenerator> rng;
};

}
