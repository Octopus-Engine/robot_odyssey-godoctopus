#include "Pickable.h"

#include "godoctopus/display/vat/VatLibraryHandle.h"

#include <iostream>

static Color color_from_idx(int idx_p)
{
	// Compute the color based on the idx
	int r = idx_p % 256;
	int g = (idx_p/ 256 ) % 256;
	int b = (idx_p/ (256*256) ) % 256;
	return Color::from_rgba8(r,g,b);
}

static int idx_from_color(Color const &color_p)
{
	if (color_p.a <= 0.01) {
		return -1;
	}
	int r = color_p.get_r8();
	int g = color_p.get_g8();
	int b = color_p.get_b8();
	if(r != 255 || g != 255 || b != 255) {
		return r + g *256 + b *256*256;
	}
	return -1;
}

static Color safe_color(int x, int y, Ref<Image> const &image_p)
{
	if(x >= 0 && x < image_p->get_width()
	&& y >= 0 && y < image_p->get_height())
	{
		return image_p->get_pixel(x, y);
	}
	return Color(1.f,1.f,1.f,0.f);
}

namespace godot {

int PickerNode::add_entity(flecs::entity e) {
	return entities.new_instance(e).handle();
}

void PickerNode::remove_entity(int idx) {
	entities.free_instance(idx);
}

void PickerNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
		} break;
	}
}

TypedArray<bool> PickerNode::index_array_from_texture(Rect2 const &rect_p) const {
	if(!texture.is_valid())
	{
		return TypedArray<bool>();
	}
	// scale from texture viewport scale
	Rect2i rect_l = Rect2i(rect_p.get_position(), rect_p.get_size());
	Ref<Image> image_l = texture->get_image();
	TypedArray<bool> all_added_l;
	all_added_l.resize(entities.size());
	all_added_l.fill(false);
	for(int32_t x = rect_l.get_position().x ; x <= rect_l.get_position().x + rect_l.get_size().x ; ++ x)
	{
		for(int32_t y = rect_l.get_position().y ; y <= rect_l.get_position().y + rect_l.get_size().y ; ++ y)
		{
			Color color = safe_color(x, y, image_l);
			int idx_l = idx_from_color(color);
			if(idx_l >= 0)
			{
				// if (idx_l >= all_added_l.size()) {
				// 	std::cout<<color.r<<" "<<color.g<<" "<<color.b<<" "<<color.a<<std::endl;
				// }
				all_added_l[idx_l] = true;
			}
		}
	}
	return all_added_l;
}

EntityGroup *PickerNode::group_from_texture(Rect2 const &rect_p) const {
	EntityGroup *group = memnew(EntityGroup);
	if(!texture.is_valid())
	{
		return group;
	}

	TypedArray<bool> set = index_array_from_texture(rect_p);
	for(int i = 0 ; i < set.size() ; ++ i) {
		if(set[i]) {
			group->get_entities().push_back(entities.get(i));
		}
	}

	return group;
}

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
