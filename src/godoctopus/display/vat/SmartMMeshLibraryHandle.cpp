#include "SmartMMeshLibraryHandle.h"

#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"

#include "octopus_types.h"

#include "DisplayVatHelpers.h"

void declare_smart_mmesh_library_systems(flecs::world &ecs, godot::SmartMMeshLibrary *library, godot::VatLibrary *vat_library, int32_t selection_multi_mesh_id) {

	// no instance id to enable reload
	ecs.component<SmartMMeshLibraryHandle>()
		.member("multi_mesh_id", &SmartMMeshLibraryHandle::multi_mesh_id)
	;

	ecs.component<ProjectileSmartMMesh>()
		.member("r", &ProjectileSmartMMesh::r)
		.member("g", &ProjectileSmartMMesh::g)
		.member("b", &ProjectileSmartMMesh::b)
		.member("a", &ProjectileSmartMMesh::a)
		.member("scale", &ProjectileSmartMMesh::scale)
		.member("up", &ProjectileSmartMMesh::up)
		.member("end_up", &ProjectileSmartMMesh::end_up)
		.member("timestamp_start", &ProjectileSmartMMesh::timestamp_start)
		.member("timestamp_end", &ProjectileSmartMMesh::timestamp_end)
	;

	ecs.component<RelativePosition>()
		.member("ref_entity", &RelativePosition::ref_entity)
		.member("x", &RelativePosition::x)
		.member("y", &RelativePosition::y)
		.member("z", &RelativePosition::z)
	;

	using fcn_projectile = std::function<void(godot::SmartMultiMeshInstance*, SmartMMeshLibraryHandle const &, ProjectileSmartMMesh&)>;
	declare_displayer_instance_handling_systems<godot::SmartMMeshLibrary, godot::SmartMultiMeshInstance, SmartMMeshLibraryHandle, ProjectileSmartMMesh>(
		ecs,
		library,
		fcn_projectile([](godot::SmartMultiMeshInstance* mmesh, SmartMMeshLibraryHandle const &handle, ProjectileSmartMMesh &proj) {
			int instance_id = handle.instance_id;
			mmesh->set_color(instance_id, Color(proj.r.to_double(), proj.g.to_double(), proj.b.to_double(), proj.a.to_double()).srgb_to_linear());
			Transform3D transform = mmesh->get_old_instance_transform(instance_id);
			transform.scale_basis(proj.scale.to_double()*Vector3(1,1,1));
			transform.origin.y = WORLD_SCALE * proj.up.to_double();
			mmesh->set_instance_transform(instance_id, transform);
		})
	);

	declare_displayer_instance_handling_systems<godot::SmartMMeshLibrary, godot::SmartMultiMeshInstance, SmartMMeshLibraryHandle, RelativePosition>(ecs,library);

	// selection handling
	using SelectionSmartMeshHandle = SmartMMeshLibraryHandleT<Selected>;
	// no instance id to enable reload
	ecs.component<SelectionSmartMeshHandle>()
		.member("multi_mesh_id", &SelectionSmartMeshHandle::multi_mesh_id)
	;

	ecs.component<Selected>()
		.member("selected", &Selected::selected)
	;

	declare_displayer_instance_handling_systems<godot::SmartMMeshLibrary, godot::SmartMultiMeshInstance, SelectionSmartMeshHandle>(ecs, library);

	// Update phase

	// update based on selection status (before locking mutex)
	ecs.system<Selected const>()
		.immediate()
		.kind(ecs.entity(DisplaySyncPhase))
		.with<SelectionSmartMeshHandle>()
		.write<SelectionSmartMeshHandle>()
		.each([ecs](flecs::entity e, Selected const &selected) {
			if (!selected.selected) {
				ecs.defer_suspend();
				e.remove<SelectionSmartMeshHandle>();
				ecs.defer_resume();
			}
		});
	ecs.system<Selected const>()
		.immediate()
		.kind(ecs.entity(DisplaySyncPhase))
		.without<SelectionSmartMeshHandle>()
		.write<SelectionSmartMeshHandle>()
		.each([ecs, selection_multi_mesh_id](flecs::entity e, Selected const &selected) {
			if (selected.selected) {
				ecs.defer_suspend();
				e.set<SelectionSmartMeshHandle>({selection_multi_mesh_id});
				ecs.defer_resume();
			}
		});

	// lock mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.lock();
			library->swap_transforms();
		});


	// update all positions
	ecs.system<octopus::Position const, SmartMMeshLibraryHandle const, ProjectileSmartMMesh const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.multi_threaded()
		.each([&ecs, library](flecs::entity e, octopus::Position const &pos, SmartMMeshLibraryHandle const &handle, ProjectileSmartMMesh const &proj) {
			if(handle.instance_id < 0 ) { return; }
			godot::SmartMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
			double time = 1.;
			if (proj.timestamp_start != proj.timestamp_end) {
				time = double(octopus::get_time_stamp(ecs) - proj.timestamp_start) / double(proj.timestamp_end - proj.timestamp_start);
				time = std::clamp(time, 0., 1.);
			}
			double up = proj.up.to_double() + (proj.end_up.to_double()-proj.up.to_double()) * time;
			mmesh->set_instance_new_position(handle.instance_id, WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), up, real_t(octopus::to_double(pos.pos.y))));
		});

	if (vat_library) {
		ecs.system<RelativePosition const, SmartMMeshLibraryHandle const>()
			.kind(ecs.entity(DisplaySyncPhase))
			.multi_threaded()
			.each([&ecs, library, vat_library](flecs::entity e, RelativePosition const &pos, SmartMMeshLibraryHandle const &handle) {
				if(!pos.ref_entity.is_valid() || !pos.ref_entity.is_alive() || !pos.ref_entity.enabled()) {
					e.destruct();
				}
				auto const vat_handle = pos.ref_entity.try_get<VatLibraryHandle>();
				if(handle.instance_id < 0 || !vat_handle || vat_handle->instance_id < 0) { return; }
				godot::SmartMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
				Vector3 position = get_transformed_position(Vector3(pos.x,pos.y,pos.z), vat_library, *vat_handle);
				mmesh->set_instance_new_position(handle.instance_id, WORLD_SCALE * position);
			});
	}

	// update all positions for selection
	ecs.system<octopus::Position const, SelectionSmartMeshHandle const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.multi_threaded()
		.each([&ecs, library](flecs::entity e, octopus::Position const &pos, SelectionSmartMeshHandle const &handle) {
			if(handle.instance_id < 0 ) { return; }
			godot::SmartMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
			mmesh->set_instance_new_position(handle.instance_id, WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), 0., real_t(octopus::to_double(pos.pos.y))));
		});

	// clear up mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.unlock();
		});

}
