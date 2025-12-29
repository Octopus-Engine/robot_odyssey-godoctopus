#include "SmartMultiMeshInstance.h"

#include "core/os/os.h"

namespace godot {

void SmartMultiMeshInstance::_ready() {
	if (!get_multimesh().is_valid()) {
		set_multimesh(memnew(MultiMesh));
	}
	auto mesh = get_multimesh();
	mesh->set_instance_count(0);
	mesh->set_transform_format(MultiMesh::TRANSFORM_3D);
	mesh->set_use_colors(true);
	mesh->set_use_custom_data(true);
}

void SmartMultiMeshInstance::_process(double delta) {
	lock();
	elapsed_time += delta;
	auto mesh = get_multimesh();
	if (mesh.is_valid() && time_step > 0.) {
		int instance_id = 0;
		data.for_each([this, &mesh, &instance_id] (MultiMeshData &d, size_t i) {
			// transform
			Transform3D transform = old_transform[i].interpolate_with(new_transform[i], elapsed_time / time_step);
			mesh->set_instance_transform(instance_id, transform);
			// color info
			mesh->set_instance_color(instance_id, d.color);
			mesh->set_instance_custom_data(instance_id, d.outline_color);
			++instance_id;
		});
		mesh->set_visible_instance_count(instance_id);
	}
	unlock();
}

int SmartMultiMeshInstance::add_instance() {
	size_t id = data.new_instance(MultiMeshData()).handle();
	if (id >= old_transform.size()) {
		old_transform.resize(id+1);
		new_transform.resize(id+1);
	}
	if (id >= get_multimesh()->get_instance_count()) {
		get_multimesh()->set_instance_count(id+1);
	}
	// reset transform
	old_transform[id] = Transform3D();
	new_transform[id] = Transform3D();
	return id;
}

void SmartMultiMeshInstance::free_instance(int instance_id) {
	data.free_instance(instance_id);
}

// Set/Update functions
void SmartMultiMeshInstance::set_color(int instance_id, Color color) {
	data[instance_id].color = color;
}

void SmartMultiMeshInstance::set_outline_color(int instance_id, Color color) {
	data[instance_id].outline_color = color;
}

void SmartMultiMeshInstance::set_instance_transform(int instance_id, Transform3D const &transform) {
	old_transform[instance_id] = transform;
	new_transform[instance_id] = transform;
}

void SmartMultiMeshInstance::set_new_instance_transform(int instance_id, Transform3D const &transform) {
	new_transform[instance_id] = transform;
}

Transform3D SmartMultiMeshInstance::get_old_instance_transform(int instance_id) const {
	return old_transform[instance_id];
}

void SmartMultiMeshInstance::swap_transforms() {
	elapsed_time = 0.;
	std::swap(new_transform, old_transform);
}

void SmartMultiMeshInstance::set_instance_new_position(int instance_id, Vector3 const &new_position) {
	set_instance_translation(instance_id, new_position - old_transform[instance_id].origin);
}

void SmartMultiMeshInstance::set_instance_translation(int instance_id, Vector3 const &translation) {
	new_transform[instance_id] = old_transform[instance_id];
	new_transform[instance_id].origin += translation;
	if (translation != Vector3()) {
		// compute rotation
		Vector3 forward = old_transform[instance_id].xform(Vector3(1,0,0)) - old_transform[instance_id].origin;
		float delta_angle = translation.signed_angle_to(-1.*forward, Vector3(0,1,0));
		new_transform[instance_id] = new_transform[instance_id].rotated_local(Vector3(0,1,0), -delta_angle);
	}
}

void SmartMultiMeshInstance::snap_rotation(int instance_id, Vector3 direction) {
	new_transform[instance_id] = old_transform[instance_id];
	// compute rotation
	Vector3 forward = old_transform[instance_id].xform(Vector3(1,0,0)) - old_transform[instance_id].origin;
	float delta_angle = direction.signed_angle_to(-1.*forward, Vector3(0,1,0));
	new_transform[instance_id] = new_transform[instance_id].rotated_local(Vector3(0,1,0), -delta_angle);
}

void SmartMultiMeshInstance::_bind_methods() {
	ADD_SIMPLE_PROP(SmartMultiMeshInstance, FLOAT, time_step);

	ClassDB::bind_method(D_METHOD("add_instance"), &SmartMultiMeshInstance::add_instance);
	ClassDB::bind_method(D_METHOD("free_instance", "instance_id"), &SmartMultiMeshInstance::free_instance);

	ClassDB::bind_method(D_METHOD("set_color", "instance_id", "color"), &SmartMultiMeshInstance::set_color);
	ClassDB::bind_method(D_METHOD("set_outline_color", "instance_id", "color"), &SmartMultiMeshInstance::set_outline_color);
	ClassDB::bind_method(D_METHOD("set_instance_transform", "instance_id", "transform"), &SmartMultiMeshInstance::set_instance_transform);
	ClassDB::bind_method(D_METHOD("set_new_instance_transform", "instance_id", "transform"), &SmartMultiMeshInstance::set_new_instance_transform);
	ClassDB::bind_method(D_METHOD("get_old_instance_transform", "instance_id"), &SmartMultiMeshInstance::get_old_instance_transform);
	ClassDB::bind_method(D_METHOD("swap_transforms"), &SmartMultiMeshInstance::swap_transforms);

	ClassDB::bind_method(D_METHOD("set_instance_translation", "instance_id", "translation"), &SmartMultiMeshInstance::set_instance_translation);
	ClassDB::bind_method(D_METHOD("set_instance_new_position", "instance_id", "position"), &SmartMultiMeshInstance::set_instance_new_position);
	ClassDB::bind_method(D_METHOD("snap_rotation", "instance_id", "direction"), &SmartMultiMeshInstance::snap_rotation);
}

void SmartMultiMeshInstance::_notification(int p_notification) {
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
