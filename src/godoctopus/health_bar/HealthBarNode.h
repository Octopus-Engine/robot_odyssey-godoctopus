#pragma once

#include "scene/gui/control.h"
#include "scene/3d/camera_3d.h"
#include "scene/resources/material.h"

#include "smart_list/smart_list.h"
#include "godot_tools.h"
#include <mutex>

namespace godot {

struct HealthBarData {
	RID rid;
	Vector3 pos;
	float ratio;
};

class HealthBarNode : public Control {
	GDCLASS(HealthBarNode, Control)

	SET_GET_NODE_PATH(Camera3D, camera);
	SET_GET_PARAM(Ref<ShaderMaterial>, bar_material);
	SET_GET_PARAM(Ref<Texture2D>, texture);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		BIND_NODE_PATH(HealthBarNode, Camera3D, camera);
		ADD_OBJECT_PROP(HealthBarNode, ShaderMaterial, bar_material);
		ADD_OBJECT_PROP(HealthBarNode, Texture2D, texture);

		ClassDB::bind_method(D_METHOD("add_health_bar"), &HealthBarNode::add_health_bar);
		ClassDB::bind_method(D_METHOD("free_health_bar", "idx"), &HealthBarNode::free_health_bar);
		ClassDB::bind_method(D_METHOD("set_bar_position", "idx", "pos"), &HealthBarNode::set_bar_position);
		ClassDB::bind_method(D_METHOD("set_bar_ratio", "idx", "ratio"), &HealthBarNode::set_bar_ratio);
	}
	// All nodes
	void init_nodes() {
		INIT_NODE_PATH(Camera3D, camera);
	}

	int add_health_bar();
	void free_health_bar(int idx);

	void set_bar_position(int idx, Vector3 pos);
	void set_bar_ratio(int idx, float ratio);

	void _process(double delta);

protected:
	void _notification(int p_notification);
private:
	smart_list<HealthBarData> bars;
	std::mutex mutex;
};

}
