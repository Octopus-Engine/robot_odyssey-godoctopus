#include "VatMultiMeshInstance.h"

#include "core/os/os.h"

namespace godot {

void VatMultiMeshInstance::_ready() {
	if (!get_multimesh().is_valid()) {
		set_multimesh(memnew(MultiMesh));
	}
	auto mesh = get_multimesh();
	mesh->set_instance_count(0);
	mesh->set_transform_format(MultiMesh::TRANSFORM_3D);
	mesh->set_use_colors(true);
	mesh->set_use_custom_data(true);
	mesh->set_instance_count(instance_count);
	if (track.is_valid() && track->get_mesh().is_valid() && track->get_material().is_valid()) {
		mesh->set_mesh(track->get_mesh());
		set_material_override(track->get_material());
	}
	// Initialize basic data
	for(int i = 0 ; i < instance_count ; ++i) {
		data.new_instance(VatInstanceData());
	}
	old_transform.resize(instance_count);
	new_transform.resize(instance_count);
}

void VatMultiMeshInstance::_process(double delta) {
	elapsed_time += delta;
	auto mesh = get_multimesh();
	if (mesh.is_valid() && time_step > 0.) {
		int instance_id = 0;
		data.for_each([this, &mesh, &instance_id] (VatInstanceData const &d, size_t i) {
			// transform
			Transform3D transform;
			transform.basis = old_transform[i].basis.lerp(new_transform[i].basis, elapsed_time / time_step);
			transform.origin = old_transform[i].origin.lerp(new_transform[i].origin, elapsed_time / time_step);
			mesh->set_instance_transform(instance_id, transform);
			// color info
			Color color_info(
				d.pickable_color.r,
				d.pickable_color.g,
				d.pickable_color.b,
				d.alt_texture ? 1.0: 0.0
			);
			mesh->set_instance_color(instance_id, color_info);
			// custom data
			Vector2i current_track = track->get_ref_tracks()[d.track_number];
			Color custom_data(
				d.track_offset,
				current_track.x,
				current_track.y,
				d.track_speed
			);
			mesh->set_instance_custom_data(instance_id, custom_data);
			++instance_id;
		});
		mesh->set_visible_instance_count(instance_id);
	}
}

int VatMultiMeshInstance::add_instance() {
	size_t id = data.new_instance(VatInstanceData()).handle();
	if (id >= old_transform.size()) {
		old_transform.resize(id+1);
		new_transform.resize(id+1);
	}
	if (id >= get_multimesh()->get_instance_count()) {
		get_multimesh()->set_instance_count(id+1);
	}
	return id;
}

void VatMultiMeshInstance::free_instance(int instance_id) {
	data.free_instance(instance_id);
}

// Set/Update functions

// Updates the current instance_id with the provided track_number (0..number_of_animation_tracks - 1)
void VatMultiMeshInstance::update_instance_track(int instance_id, int track_number, float animation_offset) {
	data[instance_id].track_number = track_number;
	data[instance_id].track_offset = animation_offset;
}

void VatMultiMeshInstance::set_pickable_color(int instance_id, Color color) {
	data[instance_id].pickable_color = color;
}

void VatMultiMeshInstance::set_speed(int instance_id, float speed) {
	data[instance_id].track_speed = speed;
}

void VatMultiMeshInstance::set_alt_texture(int instance_id, bool alt_texture) {
	data[instance_id].alt_texture = alt_texture;
}

void VatMultiMeshInstance::set_instance_transform(int instance_id, Transform3D const &transform) {
	old_transform[instance_id] = transform;
	new_transform[instance_id] = transform;
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

	ClassDB::bind_method(D_METHOD("add_instance"), &VatMultiMeshInstance::add_instance);
	ClassDB::bind_method(D_METHOD("free_instance", "instance_id"), &VatMultiMeshInstance::free_instance);

	ClassDB::bind_method(D_METHOD("update_instance_track", "instance_id", "track_number", "animation_offset"), &VatMultiMeshInstance::update_instance_track);
	ClassDB::bind_method(D_METHOD("set_pickable_color", "instance_id", "color"), &VatMultiMeshInstance::set_pickable_color);
	ClassDB::bind_method(D_METHOD("set_speed", "instance_id", "speed"), &VatMultiMeshInstance::set_speed);
	ClassDB::bind_method(D_METHOD("set_alt_texture", "instance_id", "alt_texture"), &VatMultiMeshInstance::set_alt_texture);

	ClassDB::bind_method(D_METHOD("set_instance_transform", "instance_id", "transform"), &VatMultiMeshInstance::set_instance_transform);
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
