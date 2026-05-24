#include "EntityGroup.h"

#include "godoctopus/proxy/InfoProxyNode.h"

namespace godot {

void EntityGroup::_bind_methods() {
	ClassDB::bind_method(D_METHOD("size"), &EntityGroup::size);
	ClassDB::bind_method(D_METHOD("remove_dead_entities", "proxy_node"), &EntityGroup::remove_dead_entities);
	ClassDB::bind_method(D_METHOD("filter_from_priority", "proxy_node", "player", "team"), &EntityGroup::filter_from_priority);
	ClassDB::bind_method(D_METHOD("is_populated"), &EntityGroup::is_populated);
	ClassDB::bind_method(D_METHOD("get_populated_count"), &EntityGroup::get_populated_count);
	ClassDB::bind_method(D_METHOD("get_expected_population"), &EntityGroup::get_expected_population);
	ClassDB::bind_method(D_METHOD("get_should_populate"), &EntityGroup::get_should_populate);
	ClassDB::bind_method(D_METHOD("append_to_group", "group"), &EntityGroup::append_to_group);
	ClassDB::bind_method(D_METHOD("clone"), &EntityGroup::clone);
}

void EntityGroup::remove_dead_entities(InfoProxyNode *proxy_node) {
	if (!proxy_node) {
		return;
	}
	auto data_locker = proxy_node->get_data_locker();
	if (!data_locker.is_up_to_date(this)) {
		return;
	}
	filter_group([&data_locker](flecs::entity e) {
		auto it = data_locker.proxy_map.find(e.id());
		return  it == data_locker.proxy_map.end() || !it->second.get_alive();
	});
}

void EntityGroup::filter_from_priority(InfoProxyNode *proxy_node, int player, int team) {
	if (!proxy_node) {
		return;
	}
	auto data_locker = proxy_node->get_data_locker();

	auto compute_priority = [&data_locker, player, team](flecs::entity e) {
		auto it = data_locker.proxy_map.find(e.id());
		if (it == data_locker.proxy_map.end() || !it->second.get_alive()) {
			return 7; // dead entities have lowest priority
		}
		bool is_building = it->second.get_building();
		bool same_player = it->second.get_player() == player;
		bool same_team = it->second.get_team() == team;
		int priority = 6;
		if (same_player) {
			priority = 1;
		} else if (same_team) {
			priority = 3;
		} else {
			priority = 5;
		}
		if (is_building) {
			priority += 1;
		}
		return priority;
	};
	// Find priority to keep
	// in order:
	// - same player units: 1
	// - same player buildings: 2
	// - same team units: 3
	// - same team buildings: 4
	// - enemies units: 5
	// - enemies buildings: 6
	int best_priority = 6;
	for (flecs::entity e : entities) {
		int priority = compute_priority(e);
		if (priority < best_priority) {
			best_priority = priority;
		}
		if (best_priority == 1) {
			break;
		}
	}
	filter_group([compute_priority, best_priority](flecs::entity e) {
		return compute_priority(e) > best_priority;
	});
}

// remove all entities that match to true to the filter
void EntityGroup::filter_group(std::function<bool(flecs::entity)> filter) {
	entities.erase(std::remove_if(entities.begin(), entities.end(),
		[&filter](flecs::entity e) {
			return filter(e);
		}
	), entities.end());
}

static void insert_if_not_present(std::vector<flecs::entity> &entities, flecs::entity const &e) {
	if (!e.is_valid() || !e.is_alive() || !e.enabled()) {
		return;
	}
	if (std::find(entities.begin(), entities.end(), e) == entities.end()) {
		entities.push_back(e);
	}
}

void EntityGroup::append_to_group(Ref<EntityGroup> group) {
	auto &dst_entities = get_entities();
	for(flecs::entity &e : group->get_entities()) {
		insert_if_not_present(dst_entities, e);
	}
}

Ref<EntityGroup> EntityGroup::clone() const {
	Ref<EntityGroup> new_group = Ref<EntityGroup>(memnew(EntityGroup));
	new_group->entities = entities;
	new_group->populated_count = populated_count;
	new_group->expected_population = expected_population;
	new_group->should_populate = should_populate;
	new_group->timestamp = timestamp;
	return new_group;
}

}