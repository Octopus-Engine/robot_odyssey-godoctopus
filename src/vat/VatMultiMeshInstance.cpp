#include "VatMultiMeshInstance.h"

#include "core/os/os.h"

namespace godot {

void VatMultiMeshInstance::_ready() {
	old_transform.resize(instance_count);
	new_transform.resize(instance_count);
	auto mesh = get_multimesh();
	if (mesh.is_valid()) {
		mesh->set_instance_count(0);
		mesh->set_transform_format(MultiMesh::TRANSFORM_3D);
		mesh->set_use_colors(true);
		mesh->set_use_custom_data(true);
		mesh->set_instance_count(instance_count);
		if (track.is_valid() && track->get_mesh().is_valid() && track->get_material().is_valid()) {
			mesh->set_mesh(track->get_mesh());
			set_material_override(track->get_material());
		}
	} else {
		OS::get_singleton()->printerr("VatMultiMeshInstance: No multimesh defined");
	}
	// Initialize basic values
	for(int i = 0 ; i < instance_count ; ++i) {
		set_speed(i, 1.0);
		set_alt_texture(i, false);
	}
}

void VatMultiMeshInstance::_process(double delta) {
	elapsed_time += delta;
	auto mesh = get_multimesh();
	if (mesh.is_valid() && time_step > 0.) {
		for(int i = 0 ; i < instance_count ; ++i) {
			Transform3D transform;
			transform.basis = old_transform[i].basis.lerp(new_transform[i].basis, elapsed_time / time_step);
			transform.origin = old_transform[i].origin.lerp(new_transform[i].origin, elapsed_time / time_step);
			mesh->set_instance_transform(i, transform);
		}
	}
}

// Set/Update functions

// Updates the current instance_id with the provided track_number (0..number_of_animation_tracks - 1)
void VatMultiMeshInstance::update_instance_track(int instance_id, int track_number, float animation_offset) {
	auto custom_data = get_multimesh()->get_instance_custom_data(instance_id);
	Vector2i current_track = track->get_ref_tracks()[track_number];
	custom_data.g = current_track.x;
	custom_data.b = current_track.y;
	custom_data.r = animation_offset;
	get_multimesh()->set_instance_custom_data(instance_id, custom_data);
}

void VatMultiMeshInstance::set_pickable_color(int instance_id, Color color) {
	auto custom_data = get_multimesh()->get_instance_color(instance_id);
	custom_data.r = color.r;
	custom_data.g = color.g;
	custom_data.b = color.b;
	get_multimesh()->set_instance_color(instance_id, custom_data);
}

void VatMultiMeshInstance::set_speed(int instance_id, float speed) {
	auto custom_data = get_multimesh()->get_instance_color(instance_id);
	custom_data.a = speed;
	get_multimesh()->set_instance_color(instance_id, custom_data);
}

void VatMultiMeshInstance::set_alt_texture(int instance_id, bool alt_texture) {
	auto custom_data = get_multimesh()->get_instance_custom_data(instance_id);
	custom_data.a = alt_texture ? 1.0: 0.0;
	get_multimesh()->set_instance_custom_data(instance_id, custom_data);
}

void VatMultiMeshInstance::set_new_instance_transform(int instance_id, Transform3D const &transform) {
	new_transform[instance_id] = transform;
}

Transform3D VatMultiMeshInstance::get_old_instance_transform(int instance_id) const {
	return old_transform[instance_id];
}

void VatMultiMeshInstance::swap_transforms() {
	elapsed_time = 0.;
	std::swap(new_transform, old_transform);
}

void VatMultiMeshInstance::_bind_methods() {
	ADD_OBJECT_PROP(VatMultiMeshInstance, VatAnimationTrack, track);
	ADD_SIMPLE_PROP(VatMultiMeshInstance, INT, instance_count);
	ADD_SIMPLE_PROP(VatMultiMeshInstance, FLOAT, time_step);

	ClassDB::bind_method(D_METHOD("update_instance_track", "instance_id", "track_number", "animation_offset"), &VatMultiMeshInstance::update_instance_track);
	ClassDB::bind_method(D_METHOD("set_pickable_color", "instance_id", "color"), &VatMultiMeshInstance::set_pickable_color);
	ClassDB::bind_method(D_METHOD("set_speed", "instance_id", "speed"), &VatMultiMeshInstance::set_speed);
	ClassDB::bind_method(D_METHOD("set_alt_texture", "instance_id", "alt_texture"), &VatMultiMeshInstance::set_alt_texture);

	ClassDB::bind_method(D_METHOD("set_new_instance_transform", "instance_id", "transform"), &VatMultiMeshInstance::set_new_instance_transform);
	ClassDB::bind_method(D_METHOD("get_old_instance_transform", "instance_id"), &VatMultiMeshInstance::get_old_instance_transform);
	ClassDB::bind_method(D_METHOD("swap_transforms"), &VatMultiMeshInstance::swap_transforms);
}

void VatMultiMeshInstance::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
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
