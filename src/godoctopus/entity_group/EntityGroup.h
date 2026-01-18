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
		ClassDB::bind_method(D_METHOD("is_populated"), &EntityGroup::is_populated);
		ClassDB::bind_method(D_METHOD("get_populated_count"), &EntityGroup::get_populated_count);
		ClassDB::bind_method(D_METHOD("get_expected_population"), &EntityGroup::get_expected_population);
		ClassDB::bind_method(D_METHOD("get_should_populate"), &EntityGroup::get_should_populate);
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

	void set_should_populate() {
		should_populate = true;
	}

	void increase_expected_population(uint32_t exp=1) {
		expected_population += exp;
	}

	void increase_populated() {
		++populated_count;
	}

	bool is_populated() const {
		return !should_populate || populated_count == expected_population;
	}

	uint32_t get_populated_count() const { return populated_count; }
	uint32_t get_expected_population() const { return expected_population; }
	bool get_should_populate() const { return should_populate; }
protected:
private:
	std::vector<flecs::entity> entities;
	uint32_t populated_count = 0;
	uint32_t expected_population = 0;
	bool should_populate = false;
};

}
