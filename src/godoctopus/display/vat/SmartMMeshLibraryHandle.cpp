#include "SmartMMeshLibraryHandle.h"

#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/basic/position/Position.hh"

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
	;

	declare_displayer_instance_handling_systems<godot::SmartMMeshLibrary, godot::SmartMultiMeshInstance, SmartMMeshLibraryHandle>(
		ecs,
		library,
		[](godot::SmartMultiMeshInstance* mmesh, SmartMMeshLibraryHandle const &handle) {
			int instance_id = handle.instance_id;
			mmesh->set_color(instance_id, Color(handle.r.to_double(), handle.g.to_double(), handle.b.to_double(), handle.a.to_double()).srgb_to_linear());
			Transform3D transform = mmesh->get_old_instance_transform(instance_id);
			transform.scale_basis(handle.scale.to_double()*Vector3(1,1,1));
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
		.each([library](flecs::entity e, octopus::Position const &pos, SmartMMeshLibraryHandle const &handle) {
			if(handle.instance_id < 0 ) { return; }
			godot::SmartMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
			mmesh->set_instance_new_position(handle.instance_id, WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), handle.up.to_double(), real_t(octopus::to_double(pos.pos.y))));
		});

	// clear up mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.unlock();
		});
}
