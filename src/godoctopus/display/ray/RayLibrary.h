#pragma once

#include <unordered_map>

#include "scene/3d/node_3d.h"

#include "RaySmartMMesh.h"

#include "godot_tools.h"

namespace godot {

class RayLibrary : public Node3D {
	GDCLASS(RayLibrary, Node3D)
public:
	~RayLibrary() {}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_element_from_key", "key"), &RayLibrary::get_element_from_key);
	}
	void _ready();

	RaySmartMMesh *get_element_from_key(String const &key);
	RaySmartMMesh *get_element(std::string const &key);

protected:
	bool init = false;
	void _notification(int p_notification);

	std::unordered_map<std::string, RaySmartMMesh *> _elements_map;
};

}
