#include "Pickable.h"

#include "godoctopus/display/vat/VatLibraryHandle.h"

static Color color_from_idx(int idx_p)
{
	// Compute the color based on the idx
	int r = idx_p % 256;
	int g = (idx_p/ 256 ) % 256;
	int b = (idx_p/ (256*256) ) % 256;
	return Color::from_rgba8(r,g,b);
}

void declare_pickable_systems(flecs::world &ecs, godot::VatLibrary *library, godot::PickerNode *picker) {
	ecs.component<Pickable>();
	ecs.component<PickableSetUp>();

	ecs.system<VatLibraryHandle const>()
		.with<Pickable>()
		.without<PickableSetUp>()
		.kind(ecs.entity(DisplaySyncPhase))
		.each([library, picker](flecs::entity e, VatLibraryHandle const & handle) {
			if (handle.instance_id < 0) {
				return;
			}
			std::lock_guard<std::mutex> lock(library->_mutex);
			auto mmesh = library->get_multi_mesh(handle.multi_mesh_id);
			int id = picker->add_entity(e);
			mmesh->set_pickable_color(handle.instance_id, color_from_idx(id));
			e.set<PickableSetUp>({id});
		});

	ecs.system<PickableSetUp>()
		.with(flecs::Disabled)
		.kind(ecs.entity(DisplaySyncPhase))
		.each([picker](flecs::entity e, PickableSetUp const &pickable) {
			if (pickable.id < 0) {
				return;
			}
			picker->remove_entity(pickable.id);
			e.remove<PickableSetUp>();
	});

	// Todo remove all PickableSetUp when saving
}
