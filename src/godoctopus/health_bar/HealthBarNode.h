#pragma once

#include "scene/gui/control.h"
#include "scene/3d/camera_3d.h"
#include "scene/resources/material.h"

#include "godoctopus/game/GameNode.h"

#include "smart_list/smart_list.h"
#include "godot_tools.h"
#include <mutex>

namespace godot {

struct HealthBar {
	double offset;
	double width;
	int idx_bar = -1;
};

struct HealthBarData {
	RID rid;
	Vector3 pos;
	double width;
	float ratio;
};

class HealthBarNode : public Node {
	GDCLASS(HealthBarNode, Node)

	SET_GET_NODE_PATH(Camera3D, camera);
	SET_GET_NODE_PATH(GameNode, game_node);
	SET_GET_NODE_PATH(Control, health_bar_control_container);
	SET_GET_PARAM(Ref<ShaderMaterial>, bar_material);
	SET_GET_PARAM(Ref<Texture2D>, texture);
	SET_GET_PARAM(int, display_mode);
public:
	~HealthBarNode();
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		BIND_NODE_PATH(HealthBarNode, Camera3D, camera);
		BIND_NODE_PATH(HealthBarNode, GameNode, game_node);
		BIND_NODE_PATH(HealthBarNode, Control, health_bar_control_container);
		ADD_OBJECT_PROP(HealthBarNode, ShaderMaterial, bar_material);
		ADD_OBJECT_PROP(HealthBarNode, Texture2D, texture);

		ClassDB::bind_method(D_METHOD("add_health_bar"), &HealthBarNode::add_health_bar);
		ClassDB::bind_method(D_METHOD("free_health_bar", "idx"), &HealthBarNode::free_health_bar);
		ClassDB::bind_method(D_METHOD("set_bar_position", "idx", "pos"), &HealthBarNode::set_bar_position);
		ClassDB::bind_method(D_METHOD("set_bar_ratio", "idx", "ratio"), &HealthBarNode::set_bar_ratio);
		ClassDB::bind_method(D_METHOD("setup"), &HealthBarNode::setup);

		ClassDB::bind_method(D_METHOD("set_display_mode"), &HealthBarNode::set_display_mode);
		ClassDB::bind_method(D_METHOD("get_display_mode"), &HealthBarNode::get_display_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "display_mode", PROPERTY_HINT_ENUM, "Always,Damaged,Never"), "set_display_mode", "get_display_mode");
	}
	// All nodes
	void init_nodes() {
		INIT_NODE_PATH(Camera3D, camera);
		INIT_NODE_PATH(GameNode, game_node);
		INIT_NODE_PATH(Control, health_bar_control_container);
	}

	int add_health_bar();
	void free_health_bar(int idx);

	void set_bar_position(int idx, Vector3 pos);
	void set_bar_ratio(int idx, float ratio);

	void _process(double delta);

	std::mutex mutex;
protected:
	void _notification(int p_notification);
private:
	smart_list<HealthBarData> bars;

	void setup();
};

}
