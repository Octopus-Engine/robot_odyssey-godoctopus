#include "SmartMMeshLibraryHandle.h"

#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"

#include "octopus_types.h"

#include "DisplayVatHelpers.h"

void declare_smart_mmesh_library_systems(flecs::world &ecs, godot::SmartMMeshLibrary *library) {

	// no instance id to enable reload
	ecs.component<SmartMMeshLibraryHandle>()
		.member("multi_mesh_id", &SmartMMeshLibraryHandle::multi_mesh_id)
		.member("r", &SmartMMeshLibraryHandle::r)
		.member("g", &SmartMMeshLibraryHandle::g)
		.member("b", &SmartMMeshLibraryHandle::b)
		.member("a", &SmartMMeshLibraryHandle::a)
		.member("scale", &SmartMMeshLibraryHandle::scale)
		.member("up", &SmartMMeshLibraryHandle::up)
		.member("end_up", &SmartMMeshLibraryHandle::end_up)
		.member("timestamp_start", &SmartMMeshLibraryHandle::timestamp_start)
		.member("timestamp_end", &SmartMMeshLibraryHandle::timestamp_end)
	;

	declare_displayer_instance_handling_systems<godot::SmartMMeshLibrary, godot::SmartMultiMeshInstance, SmartMMeshLibraryHandle>(
		ecs,
		library,
		[](godot::SmartMultiMeshInstance* mmesh, SmartMMeshLibraryHandle const &handle) {
			int instance_id = handle.instance_id;
			mmesh->set_color(instance_id, Color(handle.r.to_double(), handle.g.to_double(), handle.b.to_double(), handle.a.to_double()).srgb_to_linear());
			Transform3D transform = mmesh->get_old_instance_transform(instance_id);
			transform.scale_basis(handle.scale.to_double()*Vector3(1,1,1));
			transform.origin.y = WORLD_SCALE * handle.up.to_double();
			mmesh->set_instance_transform(instance_id, transform);
		}
	);

	// Update phase

	// lock mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.lock();
			library->swap_transforms();
		});

	// update all positions
	ecs.system<octopus::Position const, SmartMMeshLibraryHandle const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.multi_threaded()
		.each([&ecs, library](flecs::entity e, octopus::Position const &pos, SmartMMeshLibraryHandle const &handle) {
			if(handle.instance_id < 0 ) { return; }
			godot::SmartMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
			double time = 1.;
			if (handle.timestamp_start != handle.timestamp_end) {
				time = double(octopus::get_time_stamp(ecs) - handle.timestamp_start) / double(handle.timestamp_end - handle.timestamp_start);
				time = std::clamp(time, 0., 1.);
			}
			double up = handle.up.to_double() + (handle.end_up.to_double()-handle.up.to_double()) * time;
			mmesh->set_instance_new_position(handle.instance_id, WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), up, real_t(octopus::to_double(pos.pos.y))));
		});

	// clear up mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.unlock();
		});
}
