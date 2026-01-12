#pragma once

#include "scene/main/node.h"

#include <vector>
#include <functional>
#include "flecs.h"

namespace godot {

class EntityGroup : public Resource {
	GDCLASS(EntityGroup, Resource)
public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("size"), &EntityGroup::size);
		ClassDB::bind_method(D_METHOD("remove_dead_entities"), &EntityGroup::remove_dead_entities);
	}

	std::vector<flecs::entity> const & get_entities() const { return entities; }
	std::vector<flecs::entity> & get_entities() { return entities; }

	int size() const { return entities.size(); }
	void remove_dead_entities() {
		filter_group([](flecs::entity e) {
			return !e.is_valid() || !e.is_alive() || !e.enabled();
		});
	}

	// remove all entities that math to true to the filter
	void filter_group(std::function<bool(flecs::entity)> filter) {
		entities.erase(std::remove_if(entities.begin(), entities.end(),
			[&filter](flecs::entity e) {
				return filter(e);
			}
		), entities.end());
	}
protected:
private:
	std::vector<flecs::entity> entities;
};

}
