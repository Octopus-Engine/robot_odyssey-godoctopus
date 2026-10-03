#include "RayLibrary.h"

namespace godot {

void RayLibrary::_ready() {
	_elements_map.clear();
	for (auto const &child : get_children()) {
		RaySmartMMesh *mesh = Object::cast_to<RaySmartMMesh>(child);
		if (mesh) {
			_elements_map[mesh->get_key().utf8().get_data()] = mesh;
		}
	}
	init = true;
}

void RayLibrary::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			_ready();
			set_process(false);
			set_physics_process(false);
		} break;
	}
}

RaySmartMMesh *RayLibrary::get_element_from_key(String const &key) {
	if (!init) {
		return nullptr;
	}
	return get_element(key.utf8().get_data());
}

RaySmartMMesh *RayLibrary::get_element(std::string const &key) {
	if (!init) {
		return nullptr;
	}
	auto it = _elements_map.find(key);
	if (it != _elements_map.end()) {
		return it->second;
	}
	return nullptr;
}

} // namespace godot
