#include "VatLibraryHandle.h"

#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/basic/position/Position.hh"

#include "octopus_types.h"

#include "DisplayVatHelpers.h"

Vector3 get_transformed_position(Vector3 const &pos, godot::VatLibrary *library, VatLibraryHandle const &handle) {
	std::lock_guard<std::mutex> lock(library->_mutex);
	godot::VatMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
	Transform3D cur_transform = mmesh->get_current_instance_transform(handle.instance_id);
	return cur_transform.xform(pos);
}

void declare_vat_library_systems(flecs::world &ecs, godot::VatLibrary *library) {

	// no instance id to enable reload
	ecs.component<VatLibraryHandle>()
		.member("multi_mesh_id", &VatLibraryHandle::multi_mesh_id)
	;

	std::function<void(godot::VatMultiMeshInstance*, VatLibraryHandle const &, VatLibraryHandle&)> setup = [](godot::VatMultiMeshInstance* mmesh, VatLibraryHandle const &handle, VatLibraryHandle&) {};
	declare_displayer_instance_handling_systems<godot::VatLibrary, godot::VatMultiMeshInstance, VatLibraryHandle, VatLibraryHandle>(
		ecs,
		library,
		setup);

	// Update phase

	// lock mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.lock();
			library->swap_transforms();
		});

	// update all positions
	ecs.system<octopus::Position const, VatLibraryHandle const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.multi_threaded()
		.each([library](flecs::entity e, octopus::Position const &pos, VatLibraryHandle const &handle) {
			if(handle.instance_id < 0 ) { return; }
			godot::VatMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);
			mmesh->set_instance_new_position(handle.instance_id, WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), 0., real_t(octopus::to_double(pos.pos.y))));
		});

	// update animation when winding up
	ecs.system<octopus::Position const, octopus::Attack const, octopus::AttackCommand const, VatLibraryHandle const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.multi_threaded()
		.each([library](flecs::entity e, octopus::Position const &pos,
		octopus::Attack const &attack, octopus::AttackCommand const& attackCommand, VatLibraryHandle const &handle) {
			if(handle.instance_id < 0 ) { return; }
			godot::VatMultiMeshInstance *mmesh = library->get_multi_mesh(handle.multi_mesh_id);

			if(attack.windup == 1)
			{
				octopus::Position const * target_pos = attackCommand.target ? attackCommand.target.try_get<octopus::Position>() : nullptr;
				if(target_pos)
				{
					auto dir_l = target_pos->pos - pos.pos;
					mmesh->snap_rotation(handle.instance_id, Vector3(real_t(to_double(dir_l.x)), 0., real_t(to_double(dir_l.y))));
				}
				mmesh->update_instance_track(handle.instance_id, mmesh->get_track()->get_windup_anim(), true);
			}
		});

	// clear up mutex
	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([library](flecs::iter&) {
			library->_mutex.unlock();
		});
}
