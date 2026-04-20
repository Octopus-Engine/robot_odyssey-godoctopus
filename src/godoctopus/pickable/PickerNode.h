#pragma once

#include "scene/main/node.h"
#include "scene/main/viewport.h"

#include "godoctopus/entity_group/EntityGroup.h"

#include "smart_list/smart_list.h"

#include "flecs.h"
#include "godot_tools.h"

namespace godot {

class InfoProxyNode;

class PickerNode : public Node {
	GDCLASS(PickerNode, Node)

	SET_GET_NODE_PATH(InfoProxyNode, proxy_node);
	SET_GET_PARAM(Ref<ViewportTexture>, texture);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();
	// All nodes
	void init_nodes();

	TypedArray<bool> index_array_from_texture(Rect2 const &rect_p) const;
	Ref<EntityGroup> group_from_texture(Rect2 const &rect_p) const;
	Ref<EntityGroup> group_from_texture_and_world(Rect2 const &rect_p, TypedArray<Vector2> const &world_rect_p) const;
	Ref<EntityGroup> single_selection_from_texture(Rect2 const &rect_p) const;

	int add_entity(flecs::entity e);
	void remove_entity(int id);
protected:
	void _notification(int p_notification);
private:
	smart_list<flecs::entity> entities;
	mutable std::mutex mutex;
};

}
