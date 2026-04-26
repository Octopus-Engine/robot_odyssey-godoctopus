#pragma once

#include "scene/main/node.h"

#include <vector>
#include <functional>
#include "flecs.h"

namespace godot {

class InfoProxyNode;

class EntityGroup : public Resource {
	GDCLASS(EntityGroup, Resource)
public:
	static void _bind_methods();

	std::vector<flecs::entity> const & get_entities() const { return entities; }
	std::vector<flecs::entity> & get_entities() { return entities; }

	int size() const { return entities.size(); }

	void remove_dead_entities(InfoProxyNode *proxy_node);

	void filter_from_priority(InfoProxyNode *proxy_node, int player, int team);

	// remove all entities that match to true to the filter
	void filter_group(std::function<bool(flecs::entity)> filter);

	void set_should_populate() {
		should_populate = true;
	}

	void increase_expected_population(uint32_t exp=1) {
		expected_population += exp;
	}

	void increase_populated(int64_t current_timestamp) {
		++populated_count;
		if (timestamp == -1) {
			timestamp = current_timestamp;
		}
	}

	int64_t get_timestamp() const { return timestamp; }

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
	// the timestamp at which the group was first populated
	int64_t timestamp = -1;
};

}
