#pragma once

#include "scene/3d/node_3d.h"
#include "vat/SmartMultiMeshInstance.h"

#include "smart_list/smart_list.h"
#include "godot_tools.h"
#include "core/math/random_number_generator.h"

namespace godot {

class ParticuleTypeResource : public Resource {
	GDCLASS(ParticuleTypeResource, Resource)

	SET_GET_PARAM(Ref<Curve>, scale_curve);
	SET_GET_PARAM(Ref<Curve>, speed_curve);
	SET_GET_PARAM(Ref<Curve>, rotation_curve);
	SET_GET_PARAM(Ref<Curve>, custom_data_x);
	SET_GET_PARAM(Ref<Curve>, custom_data_y);
	SET_GET_PARAM(Ref<Curve>, custom_data_z);
	SET_GET_PARAM(Ref<Curve>, custom_data_w);
	SET_GET_PARAM_DEF(double, time, 1.0);
	SET_GET_PARAM_DEF(int, mesh_idx, 0);
public:
	ParticuleTypeResource() {}
	~ParticuleTypeResource() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, scale_curve);
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, speed_curve);
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, rotation_curve);
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, custom_data_x);
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, custom_data_y);
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, custom_data_z);
		ADD_OBJECT_PROP(ParticuleTypeResource, Curve, custom_data_w);
		ADD_SIMPLE_PROP(ParticuleTypeResource, FLOAT, time);
		ADD_SIMPLE_PROP(ParticuleTypeResource, INT, mesh_idx);
	}
};

struct ParticuleTypeData {
	Color color;
	Vector3 scale = Vector3(1,1,1);
	int resource = -1;
	std::vector<Vector3> position;
	std::vector<Vector3> direction;
	std::vector<double> time_offset;
	std::vector<double> lifetime;
};

class ParticleOrchestrator : public MultiMeshInstance3D {
	GDCLASS(ParticleOrchestrator, MultiMeshInstance3D)

	SET_GET_PARAM(TypedArray<Ref<ParticuleTypeResource>>, particule_resources);

public:
	~ParticleOrchestrator() {}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_ARRAY_OBJECT_PROP(ParticleOrchestrator, ParticuleTypeResource, particule_resources);

		ClassDB::bind_method(D_METHOD("add_instance", "pos", "color", "resource"), &ParticleOrchestrator::add_instance);
	}
	void _ready();
	void _process(double delta);

	void add_instance(Vector3 const &pos, Color const &color, int resource);

protected:
	std::mutex _mutex;
	void _notification(int p_notification);

	smart_list<ParticuleTypeData> data;
	double elapsed = 0.;
	Ref<RandomNumberGenerator> rng;
	std::vector<MultiMeshInstance3D*> mesh_instances;

	MultiMeshInstance3D* get_mmseh_or_default(int mesh_idx) {
		if (mesh_idx >= 0 && static_cast<size_t>(mesh_idx) < mesh_instances.size()) {
			return mesh_instances[mesh_idx];
		}
		return this;
	}

	std::size_t get_offseted_mesh_idx(int mesh_idx) {
		if (mesh_idx >= 0 && static_cast<size_t>(mesh_idx) < mesh_instances.size()) {
			return mesh_idx+1;
		}
		return 0;
	}
};

}
