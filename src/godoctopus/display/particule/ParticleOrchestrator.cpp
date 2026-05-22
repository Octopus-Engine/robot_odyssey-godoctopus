#include "ParticleOrchestrator.h"
#include <iostream>
namespace godot {

void ParticleOrchestrator::_ready() {
	rng = memnew(RandomNumberGenerator);

	mesh_instances.clear();
	// iterate over children to find MultiMeshInstance3D
	for (auto child : get_children()) {
		auto mmesh = Object::cast_to<MultiMeshInstance3D>(child);
		if (mmesh) {
			mesh_instances.push_back(mmesh);
		}
	}
}

static Ref<ParticuleTypeResource> get_resource_or_default(ParticleOrchestrator const *self, int resource_idx) {
	if (resource_idx >= 0 && resource_idx < self->get_particule_resources().size()) {
		return self->get_particule_resources()[resource_idx];
	}
	return Ref<ParticuleTypeResource>();
}

void ParticleOrchestrator::_process(double delta) {
	std::lock_guard<std::mutex> lock(_mutex);
	auto mesh = get_multimesh();
	if (!mesh.is_valid()) {
		return;
	}
	std::vector<int> sizes(mesh_instances.size()+1, 0);
	data.for_each_const([&sizes, this](auto const &d) {
		auto res = get_resource_or_default(this, d.resource);
		int mesh_idx = res->get_mesh_idx();
		sizes[get_offseted_mesh_idx(mesh_idx)] += 1;
	});
	if (mesh->get_instance_count() < sizes[0]) {
		mesh->set_instance_count(sizes[0]);
	}
	mesh->set_visible_instance_count(sizes[0]);
	for(size_t i = 0; i < mesh_instances.size(); ++ i) {
		auto mmesh = mesh_instances[i];
		if (mmesh->get_multimesh()->get_instance_count() < sizes[i+1]) {
			mmesh->get_multimesh()->set_instance_count(sizes[i+1]);
		}
		mmesh->get_multimesh()->set_visible_instance_count(sizes[i+1]);
	}

	elapsed += delta;
	std::vector<int> mesh_instance_ids(mesh_instances.size()+1, 0);
	data.for_each([this, &mesh_instance_ids, &delta](ParticuleTypeData &d, size_t idx) {
		bool do_not_free = false;
		auto current_resource = get_resource_or_default(this, d.resource);
		auto cur_multi_mesh = get_mmseh_or_default(current_resource->get_mesh_idx());
		auto cur_mesh = cur_multi_mesh->get_multimesh();
		int &instance_id = mesh_instance_ids[get_offseted_mesh_idx(current_resource->get_mesh_idx())];
		double time = current_resource->get_time();
		if (d.time_offset <= elapsed && d.time_offset+time >= elapsed) {
			double lifetime = elapsed - d.time_offset;
			lifetime /= time;

			Transform3D transform;
			transform.basis.scale(d.scale * current_resource->get_scale_curve()->sample_baked(lifetime));
			d.position += d.direction * current_resource->get_speed_curve()->sample_baked(lifetime) * delta;
			transform.origin = d.position;
			transform = transform.rotated_local(Vector3(0,1,0), d.rot_y + current_resource->get_rotation_curve()->sample_baked(lifetime));
			cur_mesh->set_instance_transform(instance_id, transform);
			cur_mesh->set_instance_color(instance_id, d.color);
			Color custom_data(
				current_resource->get_custom_data_x()->sample_baked(lifetime),
				current_resource->get_custom_data_y()->sample_baked(lifetime),
				current_resource->get_custom_data_z()->sample_baked(lifetime),
				current_resource->get_custom_data_w()->sample_baked(lifetime)
			);
			cur_mesh->set_instance_custom_data(instance_id, custom_data);
			++instance_id;
			// one instance is enough to keep this data
			do_not_free = true;
		}
		do_not_free |= d.time_offset > elapsed;
		// no instance was alive -> free
		if (!do_not_free) {
			data.free_instance(idx);
		}
	});
}

void ParticleOrchestrator::add_instance(Vector3 const &pos, Color const &color, int resource) {
	std::lock_guard<std::mutex> lock(_mutex);
	ParticuleTypeData particule_data {
		color.srgb_to_linear(),		// color
		Vector3(1.,1.,1.),			// scale
		resource,					// resource
		pos,						// position
		Vector3(0.,0.,0.),			// direction
		elapsed						// time_offset
	};
	data.new_instance(std::move(particule_data));
}

void ParticleOrchestrator::add_particle(ParticuleTypeData &&particule_data) {
	particule_data.time_offset += elapsed;
	data.new_instance(std::move(particule_data));
}

void ParticleOrchestrator::_notification(int p_notification)
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
			set_physics_process(false);
		} break;
	}
}

}
