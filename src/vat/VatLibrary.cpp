#include "VatLibrary.h"

namespace godot {

void VatLibrary::_ready() {
	for (Ref<VatAnimationTrack> const &track : tracks) {
		VatMultiMeshInstance * multi_mesh = memnew(VatMultiMeshInstance);
		multi_mesh->set_time_step(time_step);
		multi_mesh->set_track(track);
		multi_mesh->set_layer_mask_value(1, true);
		multi_mesh->set_layer_mask_value(5, true);
		multi_mesh->set_layer_mask_value(6, true);
		multi_mesh->_mutex = &_mutex;
		add_child(multi_mesh);
		vec_multi_mesh.push_back(multi_mesh);
	}
}

void VatLibrary::swap_transforms() {
	for (auto &mmesh : vec_multi_mesh) {
		mmesh->swap_transforms();
	}
}

void VatLibrary::_notification(int p_notification)
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
