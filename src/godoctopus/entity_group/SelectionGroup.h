#pragma once

#include "scene/main/node.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/entity_group/EntityGroup.h"

#include "godot_tools.h"
#include <mutex>

namespace godot {


class SelectionGroup : public Node {
	GDCLASS(SelectionGroup, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:
	SelectionGroup() : _group(memnew(EntityGroup)) {}
	~SelectionGroup() {}
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		BIND_NODE_PATH(SelectionGroup, GameNode, game_node);

		ClassDB::bind_method(D_METHOD("set_group", "group"), &SelectionGroup::set_group);
		ClassDB::bind_method(D_METHOD("append_to_group", "group"), &SelectionGroup::append_to_group);
		ClassDB::bind_method(D_METHOD("delete_from_group", "group"), &SelectionGroup::delete_from_group);
		ClassDB::bind_method(D_METHOD("clear_group"), &SelectionGroup::clear_group);

		ClassDB::bind_method(D_METHOD("group"), &SelectionGroup::group);
	}
	// All nodes
	void init_nodes() {
		INIT_NODE_PATH(GameNode, game_node);
	}

	void set_group(Ref<EntityGroup> group);
	void append_to_group(Ref<EntityGroup> group);
	void delete_from_group(Ref<EntityGroup> group);
	void clear_group();
	Ref<EntityGroup> group() const;

protected:
	void _notification(int p_notification);
private:
	mutable std::mutex _mutex;
	enum class UpdateType {
		NONE,
		ADD,
		REMOVE,
		SET,
		CLEAR
	};
	UpdateType _update_type = UpdateType::NONE;
	Ref<EntityGroup> _new_group;
	Ref<EntityGroup> _group;

	void setup();
};

}
