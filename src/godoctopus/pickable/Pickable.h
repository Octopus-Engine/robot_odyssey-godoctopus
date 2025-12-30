#pragma once

#include "scene/3d/node_3d.h"
#include "scene/main/viewport.h"

#include "vat/VatLibrary.h"

#include "flecs.h"
#include "godot_tools.h"
#include "octopus_types.h"

struct Pickable { bool decoy=false; };
struct PickableSetUp { int id; };

namespace godot {

class EntityGroup : public Object {
	GDCLASS(EntityGroup, Object)
public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("size"), &EntityGroup::size);
	}

	std::vector<flecs::entity> const & get_entities() const { return entities; }
	std::vector<flecs::entity> & get_entities() { return entities; }

	int size() const { return entities.size(); }
protected:
private:
	std::vector<flecs::entity> entities;
};

class PickerNode : public Node {
	GDCLASS(PickerNode, Node)

	SET_GET_PARAM(Ref<ViewportTexture>, texture);
public:

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		ADD_OBJECT_PROP(PickerNode, ViewportTexture, texture);

		ClassDB::bind_method(D_METHOD("index_array_from_texture", "rect"), &PickerNode::index_array_from_texture);
		ClassDB::bind_method(D_METHOD("group_from_texture", "rect"), &PickerNode::group_from_texture);

	}
	// All nodes
	void init_nodes() {}

	TypedArray<bool> index_array_from_texture(Rect2 const &rect_p) const;
	EntityGroup *group_from_texture(Rect2 const &rect_p) const;

	int add_entity(flecs::entity e);
	void remove_entity(int id);
protected:
	void _notification(int p_notification);
private:
	smart_list<flecs::entity> entities;
};

}

void declare_pickable_systems(flecs::world &ecs, godot::VatLibrary *library, godot::PickerNode *picker);
