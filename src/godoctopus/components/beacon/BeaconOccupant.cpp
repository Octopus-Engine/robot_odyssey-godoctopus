#include "BeaconOccupant.h"
#include "BeaconSlotOccupied.h"

#include "octopus/components/basic/hitpoint/Destroyable.hh"

void declare_beacon_occupant_component(flecs::world &ecs)
{
	ecs.component<BeaconOccupant>();
}

void declare_beacon_occupant_observers(flecs::world &ecs)
{
	ecs.observer<BeaconOccupant const>()
		.event(flecs::OnSet)
		.each([](flecs::entity e, BeaconOccupant const &occupant) {
			if (!occupant.slot_entity.is_alive()) {
				return;
			}
			BeaconSlotOccupied *slot = occupant.slot_entity.try_get_mut<BeaconSlotOccupied>();
			if (slot) {
				slot->occupied = true;
			}
		});

	ecs.observer<octopus::Destroyable const, BeaconOccupant const>()
		.event<octopus::Destroyed>()
		.each([](flecs::entity e, octopus::Destroyable const&, BeaconOccupant const &occupant) {
			if (!occupant.slot_entity.is_alive()) {
				return;
			}
			BeaconSlotOccupied *slot = occupant.slot_entity.try_get_mut<BeaconSlotOccupied>();
			if (slot) {
				slot->occupied = false;
			}
		});
}
