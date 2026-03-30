#include "SmartMMeshLibrary.h"

namespace godot {

void SmartMMeshLibrary::_ready() {
	for (int i = 0 ; i < smarts.size() ; ++ i) {
		Object * obj = smarts[i];
		if (obj) {
			vec_multi_mesh.push_back(Object::cast_to<SmartMultiMeshInstance>(obj));
			vec_multi_mesh.back()->_mutex = &_mutex;
			vec_multi_mesh.back()->set_time_step(vec_multi_mesh.back()->get_refresh_factor() * time_step);
		}
	}
}

void SmartMMeshLibrary::swap_transforms(int64_t time_stamp) {
	for (auto &mmesh : vec_multi_mesh) {
		if ( vec_multi_mesh.back()->get_refresh_factor() > 0 && time_stamp % mmesh->get_refresh_factor() == 0) {
			mmesh->swap_transforms();
		}
	}
}

void SmartMMeshLibrary::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			//_process(get_process_delta_time());
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			//_physics_process(get_physics_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			_ready();
			set_process(true);
			set_physics_process(true);
		} break;
	}
}

}
