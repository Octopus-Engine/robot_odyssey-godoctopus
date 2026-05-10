#pragma once

#include "flecs.h"

#include "octopus/systems/phases/Phases.hh"
#include "octopus/components/basic/position/Position.hh"

template<typename Library, typename PosDisplayer, typename IndexContainer, typename... Data>
void declare_displayer_instance_handling_systems(flecs::world &ecs, Library *lib,
	std::function<void(PosDisplayer*, IndexContainer const &, Data&...)> setup) {

	ecs.observer<octopus::Position const, IndexContainer, Data...>()
		.event(flecs::OnSet)
		.each([lib, setup](flecs::entity e, octopus::Position const &pos, IndexContainer & handle, Data&... data) {
			if (handle.instance_id >= 0) {
				return;
			}
			lib->_mutex.lock();
			PosDisplayer *mmesh = lib->get_multi_mesh(handle.multi_mesh_id);
			int instance_id = mmesh->add_instance();
			// we get the old transform to keep scale
			Transform3D transform = mmesh->get_old_instance_transform(instance_id);
			transform.set_origin(WORLD_SCALE * Vector3(pos.pos.x.to_double(), 0., pos.pos.y.to_double()));
			mmesh->set_instance_transform(instance_id, transform);
			handle.instance_id = instance_id;
			setup(mmesh, handle, data...);
			lib->_mutex.unlock();
		});

	ecs.observer<IndexContainer>()
		.event(flecs::OnRemove)
		.each([lib](flecs::entity e, IndexContainer &handle) {
			if (handle.instance_id < 0) {
				return;
			}
			lib->_mutex.lock();
			PosDisplayer *mmesh = lib->get_multi_mesh(handle.multi_mesh_id);
			mmesh->free_instance(handle.instance_id);
			handle.instance_id = -1;
			lib->_mutex.unlock();
		});

	ecs.system<IndexContainer>()
		.with(flecs::Disabled)
		.kind(ecs.entity(DisplaySyncPhase))
		.each([lib](flecs::entity e, IndexContainer &handle) {
			if (handle.instance_id < 0) {
				return;
			}
			lib->_mutex.lock();
			PosDisplayer *mmesh = lib->get_multi_mesh(handle.multi_mesh_id);
			mmesh->free_instance(handle.instance_id);
			handle.instance_id = -1;
			lib->_mutex.unlock();
	});
}
