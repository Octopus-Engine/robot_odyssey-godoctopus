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
	if (mesh->get_instance_count() < data.size() * count) {
		mesh->set_instance_count(data.size() * count);
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
				transform.basis.scale(Vector3(1,1,1) * scale_curve->sample_baked(lifetime));
				d.position[i] += d.direction[i] * speed_curve->sample_baked(lifetime) * delta;
				transform.origin = d.position[i];
				mesh->set_instance_transform(instance_id, transform);

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

void ParticuleSmartMMesh::add_instance(Vector3 const &pos) {
	std::lock_guard<std::mutex> lock(_mutex);
	ParticuleData particule_data;
	particule_data.position.reserve(count);
	particule_data.direction.reserve(count);
	particule_data.time_offset.reserve(count);
	for (int i = 0 ; i < count ; ++ i) {
		particule_data.position.push_back(pos + scatter * Vector3(rng->randf_range(-1., 1.), rng->randf_range(-1., 1.), rng->randf_range(-1., 1.)));
		particule_data.direction.push_back(Vector3(rng->randf_range(-0.5, 0.5), rng->randf_range(-0.5, 0.5), rng->randf_range(-0.5, 0.5)));
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
