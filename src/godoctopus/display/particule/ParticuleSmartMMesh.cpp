#include "ParticuleSmartMMesh.h"

namespace godot {

void ParticuleSmartMMesh::_ready() {
	rng = memnew(RandomNumberGenerator);
}

void ParticuleSmartMMesh::_process(double delta) {
	std::lock_guard<std::mutex> lock(_mutex);
	auto mesh = get_multimesh();
	if (!mesh.is_valid()) {
		return;
	}
	int size = 0;
	data.for_each_const([&size](auto const &d) {
		size += (int)d.position.size();
	});
	if (mesh->get_instance_count() < size) {
		mesh->set_instance_count(size);
	}

	elapsed += delta;
	int instance_id = 0;
	data.for_each([this, &instance_id, &mesh, &delta](ParticuleData &d, size_t idx) {
		bool do_not_free = false;
		for (size_t i = 0; i < d.position.size() ; ++ i) {
			if (d.time_offset[i] >= elapsed) {
				double lifetime = d.time_offset[i] - elapsed;
				lifetime /= time;

				Transform3D transform;
				transform.basis.scale(d.scale * scale_curve->sample_baked(lifetime));
				d.position[i] += d.direction[i] * speed_curve->sample_baked(lifetime) * delta;
				transform.origin = d.position[i];
				mesh->set_instance_transform(instance_id, transform);
				mesh->set_instance_color(instance_id, d.color);
				mesh->set_instance_custom_data(instance_id, d.color);

				++instance_id;
				// one instance is enough to keep this data
				do_not_free |= d.time_offset[i]+time >= elapsed;
			}
		}
		// no instance was alive -> free
		if (!do_not_free) {
			data.free_instance(idx);
		}
	});
	mesh->set_visible_instance_count(instance_id);
}

void ParticuleSmartMMesh::add_instance(Vector3 const &pos, Color const &color) {
	add_instance_detailed(pos, color, count, Vector3(1,1,1));
}

void ParticuleSmartMMesh::add_instance_detailed(Vector3 const &pos, Color const &color, int c, Vector3 const &scale) {
	std::lock_guard<std::mutex> lock(_mutex);
	ParticuleData particule_data {color.srgb_to_linear(), scale};
	particule_data.position.reserve(c);
	particule_data.direction.reserve(c);
	particule_data.time_offset.reserve(c);
	for (int i = 0 ; i < c ; ++ i) {
		particule_data.position.push_back(pos + scatter * Vector3(rng->randf_range(-1., 1.), rng->randf_range(-1., 1.), rng->randf_range(-1., 1.)));
		particule_data.direction.push_back(Vector3(rng->randf_range(-0.5, 0.5), rng->randf_range(-0.5, 0.5), rng->randf_range(-0.5, 0.5)));
		particule_data.time_offset.push_back(elapsed + rng->randf_range(0., 0.5));
	}
	data.new_instance(std::move(particule_data));
}

void ParticuleSmartMMesh::add_instance_coned(Vector3 const &pos, Color const &color, int c, Vector3 const &scale, Vector3 const &direction, float angle_spread) {
	std::lock_guard<std::mutex> lock(_mutex);
	ParticuleData particule_data {color.srgb_to_linear(), scale};
	particule_data.position.reserve(c);
	particule_data.direction.reserve(c);
	particule_data.time_offset.reserve(c);
	for (int i = 0 ; i < c ; ++ i) {
		Vector3 dir = direction.normalized();
		float rad_angle = Math::deg_to_rad(angle_spread);
		dir = dir.rotated(Vector3(1,0,0), rng->randf_range(-rad_angle, rad_angle));
		dir = dir.rotated(Vector3(0,1,0), rng->randf_range(-rad_angle, rad_angle));
		dir = dir.rotated(Vector3(0,0,1), rng->randf_range(-rad_angle, rad_angle));
		particule_data.position.push_back(pos + scatter * dir);
		particule_data.direction.push_back(dir);
		particule_data.time_offset.push_back(elapsed + rng->randf_range(0., 0.5));
	}
	data.new_instance(std::move(particule_data));
}

void ParticuleSmartMMesh::_notification(int p_notification)
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
