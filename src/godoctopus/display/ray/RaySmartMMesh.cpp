#include "RaySmartMMesh.h"
#include <iostream>

namespace godot {

void RaySmartMMesh::_ready() {
	rng = memnew(RandomNumberGenerator);
}

static Ref<RaySmartResource> get_resource_or_default(RaySmartMMesh const *self, int resource_idx) {
	if (resource_idx >= 0 && resource_idx < self->get_ray_resources().size()) {
		return self->get_ray_resources()[resource_idx];
	} else {
		return self->get_default_resource();
	}
}

void RaySmartMMesh::_process(double delta) {
	std::lock_guard<std::mutex> lock(_mutex);
	auto mesh = get_multimesh();
	if (!mesh.is_valid()) {
		return;
	}
	int size = 0;
	data.for_each_const([&size](auto const &d) {
		++size;
	});
	if (mesh->get_instance_count() < size) {
		mesh->set_instance_count(size);
	}

	elapsed += delta;
	int instance_id = 0;
	data.for_each([this, &instance_id, &mesh, &delta](RayData &d, size_t idx) {
		auto current_resource = get_resource_or_default(this, d.resource);
		double time = current_resource->get_time();
		if (d.lifetime <= time) {
			d.lifetime += delta;
			const double lifetime = d.lifetime / time;

			const double cur_width = current_resource->get_width()->sample_baked(lifetime);
			mesh->set_instance_transform(instance_id, d.transform.scaled_local(Vector3(cur_width, 1., cur_width)));
			mesh->set_instance_color(instance_id, d.color);
			Color custom_data(
				current_resource->get_progress()->sample_baked(lifetime),
				current_resource->get_size()->sample_baked(lifetime)/d.size,
				current_resource->get_instance_data()->sample_baked(lifetime),
				0.
			);
			mesh->set_instance_custom_data(instance_id, custom_data);

			++instance_id;
		} else {
			data.free_instance(idx);
		}
	});
	mesh->set_visible_instance_count(instance_id);
}

void RaySmartMMesh::add_instance(Vector3 const &pos, Vector3 const &target, Color const &color) {
	add_instance_custom(pos, target, color, -1);
}

void RaySmartMMesh::add_instance_custom(Vector3 const &pos, Vector3 const &target, Color const &color, int resource) {
	Transform3D tr = Transform3D(Basis().looking_at(target - pos), pos);
	auto current_resource = get_resource_or_default(this, resource);
	tr = tr.rotated_local(Vector3::LEFT, Math::PI/2.);
	tr = tr.scaled_local(Vector3(1., (target - pos).length(), 1.));
	tr = tr.translated((target - pos)/2.);

	std::lock_guard<std::mutex> lock(_mutex);
	RayData ray_data {
		color.srgb_to_linear(),
		std::move(tr),
		Vector3(1,1,1),
		resource,
		0.,
		current_resource->get_size_absolute() ? (target - pos).length() : 1.
	};
	data.new_instance(std::move(ray_data));
}

void RaySmartMMesh::_notification(int p_notification)
{
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
