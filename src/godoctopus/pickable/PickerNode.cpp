#include "PickerNode.h"

#include "godoctopus/proxy/InfoProxyNode.h"

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

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void PickerNode::_bind_methods() {
	BIND_NODE_PATH(PickerNode, InfoProxyNode, proxy_node);
	ADD_OBJECT_PROP(PickerNode, ViewportTexture, texture);

	ClassDB::bind_method(D_METHOD("index_array_from_texture", "rect"), &PickerNode::index_array_from_texture);
	ClassDB::bind_method(D_METHOD("group_from_texture", "rect"), &PickerNode::group_from_texture);
	ClassDB::bind_method(D_METHOD("group_from_texture_and_world", "rect", "world_rect"), &PickerNode::group_from_texture_and_world);
	ClassDB::bind_method(D_METHOD("single_selection_from_texture", "rect"), &PickerNode::single_selection_from_texture);
}

int PickerNode::add_entity(flecs::entity e) {
	std::lock_guard<std::mutex> lock(mutex);
	return entities.new_instance(e).handle();
}

void PickerNode::remove_entity(int idx) {
	std::lock_guard<std::mutex> lock(mutex);
	entities.free_instance(idx);
}

void PickerNode::init_nodes() {
	INIT_NODE_PATH(InfoProxyNode, proxy_node);
}

void PickerNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
			init_nodes();
		} break;
	}
}

TypedArray<bool> PickerNode::index_array_from_texture(Rect2 const &rect_p) const {
	if(!texture.is_valid()) {
		return TypedArray<bool>();
	}
	// scale from texture viewport scale
	Rect2i rect_l = Rect2i(rect_p.get_position(), rect_p.get_size());
	Ref<Image> image_l = texture->get_image();
	TypedArray<bool> all_added_l;
	{
		std::lock_guard<std::mutex> lock(mutex);
		all_added_l.resize(entities.size());
		all_added_l.fill(false);
	}
	for(int32_t x = rect_l.get_position().x ; x < rect_l.get_position().x + rect_l.get_size().x ; ++ x) {
		for(int32_t y = rect_l.get_position().y ; y < rect_l.get_position().y + rect_l.get_size().y ; ++ y) {
			Color color = safe_color(x, y, image_l);
			int idx_l = idx_from_color(color);
			if(idx_l >= 0) {
				all_added_l[idx_l] = true;
			}
		}
	}
	return all_added_l;
}

Ref<EntityGroup> PickerNode::group_from_texture(Rect2 const &rect_p) const {
	Ref<EntityGroup> group(memnew(EntityGroup));

	TypedArray<bool> set = index_array_from_texture(rect_p);
	std::lock_guard<std::mutex> lock(mutex);
	for(int i = 0 ; i < set.size() ; ++ i) {
		if(set[i]) {
			group->get_entities().push_back(entities.get(i));
		}
	}

	return group;
}

static bool in_bounding_box(Rect2 const &bbox, octopus::Vector const &pos)
{
	float x = (float)pos.x.to_double();
	float y = (float)pos.y.to_double();
	return bbox.has_point(Vector2(x, y));
}

static Rect2 bounding_box_from_world_rect(TypedArray<Vector2> const &world_rect_p)
{
	if (world_rect_p.is_empty()) {
		return Rect2();
	}
	Vector2 first = world_rect_p[0];
	Rect2 bbox(first, Vector2());
	for (int i = 1; i < world_rect_p.size(); ++i) {
		bbox.expand_to(world_rect_p[i]);
	}
	return bbox;
}

static octopus::Vector g2o_vector(Vector2 const &v)
{
	return octopus::Vector(v.x, v.y);
}

Ref<EntityGroup> PickerNode::group_from_texture_and_world(Rect2 const &rect_p, TypedArray<Vector2> const &world_rect_p) const {
	if (!_proxy_node) {
		return group_from_texture(rect_p);
	}
	TypedArray<bool> set = index_array_from_texture(rect_p);

	InfoProxyNodeDataLocker locker = _proxy_node->get_data_locker();
	// Compute bounding box of the world rect (4 points defining a concave polygon)
	Rect2 bbox = bounding_box_from_world_rect(world_rect_p);

	for (auto &[entity_id, entity_data] : locker.proxy_map) {
		const int pickable_id = entity_data.get_pickable_id();
		if (pickable_id >= set.size()
		 || pickable_id < 0
		 || set[pickable_id]) {
			continue;
		}
		octopus::Vector pos = entity_data.get_position();
		// For each entity in the set, check if its position is out of the bounding box
		if (!in_bounding_box(bbox, pos)) {
			continue;
		}
		bool old_is_pos = false;
		bool has_sign = false;
		bool is_inside = true;
		for (int i = 0; i < 4; ++i) {
			octopus::Vector v = g2o_vector(world_rect_p.get((i+1) % 4)) - g2o_vector(world_rect_p.get(i));
			octopus::Vector d = pos - g2o_vector(world_rect_p.get(i));
			double cross = v.x.to_double() * d.y.to_double() - v.y.to_double() * d.x.to_double();
			bool is_pos = cross > 1e-5; // cross product sign
			if (abs(cross) > 1e-5 && has_sign && is_pos != old_is_pos) {
				// If the sign changes, the point is outside the polygon
				is_inside = false;
				break;
			}
			old_is_pos = is_pos;
			has_sign = true;
		}

		if (is_inside) {
			set[pickable_id] = true;
		}
	}


	Ref<EntityGroup> group(memnew(EntityGroup));
	std::lock_guard<std::mutex> lock(mutex);
	for(int i = 0 ; i < set.size() ; ++ i) {
		if(set[i]) {
			group->get_entities().push_back(entities.get(i));
		}
	}

	return group;
}

Ref<EntityGroup> PickerNode::single_selection_from_texture(Rect2 const &rect_p) const {
	Ref<EntityGroup> group(memnew(EntityGroup));
	if (!texture.is_valid()) {
		return group;
	}

	Rect2i rect_l = Rect2i(rect_p.get_position(), rect_p.get_size());
	Ref<Image> image_l = texture->get_image();

	const int32_t center_x = rect_l.get_position().x + rect_l.get_size().x / 2;
	const int32_t center_y = rect_l.get_position().y + rect_l.get_size().y / 2;

	int best_idx = -1;
	int64_t best_dist2 = 0;

	for (int32_t x = rect_l.get_position().x; x < rect_l.get_position().x + rect_l.get_size().x; ++x) {
		for (int32_t y = rect_l.get_position().y; y < rect_l.get_position().y + rect_l.get_size().y; ++y) {
			const int idx_l = idx_from_color(safe_color(x, y, image_l));
			if (idx_l < 0) {
				continue;
			}

			const int64_t dx = int64_t(x) - int64_t(center_x);
			const int64_t dy = int64_t(y) - int64_t(center_y);
			const int64_t dist2 = dx * dx + dy * dy;

			if (best_idx < 0 || dist2 < best_dist2) {
				best_idx = idx_l;
				best_dist2 = dist2;
			}
		}
	}

	std::lock_guard<std::mutex> lock(mutex);
	if (best_idx >= 0 && (unsigned int)best_idx < entities.size()) {
		group->get_entities().push_back(entities.get(best_idx));
	}

	return group;
}

}
