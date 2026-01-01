#pragma once

#include "scene/main/node.h"

#include <vector>
#include <functional>
#include "flecs.h"

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

	// remove all entities that math to true to the filter
	void filter_group(std::function<bool(flecs::entity)> filter);
protected:
private:
	std::vector<flecs::entity> entities;
};

}
