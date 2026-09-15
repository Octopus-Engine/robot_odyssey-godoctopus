#pragma once

#include <unordered_map>

#include "scene/3d/node_3d.h"

#include "ParticuleSmartMMesh.h"

#include "godot_tools.h"

namespace godot {

class ParticleLibrary : public Node3D {
	GDCLASS(ParticleLibrary, Node3D)
public:
	~ParticleLibrary() {}

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_element_from_key", "key"), &ParticleLibrary::get_element_from_key);
	}
	void _ready();

	ParticuleSmartMMesh *get_element_from_key(String const &key);
	ParticuleSmartMMesh *get_element(std::string const &key);

protected:
	bool init = false;
	void _notification(int p_notification);

	std::unordered_map<std::string, ParticuleSmartMMesh *> _elements_map;
};

}
