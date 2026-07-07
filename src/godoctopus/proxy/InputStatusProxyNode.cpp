#include "InputStatusProxyNode.h"

#include <algorithm>

#include "octopus/world/ability/AbilityTemplateLibrary.hh"
#include "octopus/world/production/ProductionTemplateLibrary.hh"

namespace godot {

namespace {

Ref<InputStatusResource> _make_status_with_reason(const char *reason) {
	Ref<InputStatusResource> status_resource(memnew(InputStatusResource));
	status_resource->set_ok(false);
	TypedArray<String> explanations;
	explanations.append(String(reason));
	status_resource->set_other_explanations(explanations);
	return status_resource;
}

Ref<InputStatusResource> _convert_input_status(octopus::InputStatus const &status) {
	Ref<InputStatusResource> status_resource(memnew(InputStatusResource));
	status_resource->set_ok(status.ok);
	status_resource->set_entity_id((int64_t)status.entity.id());
	status_resource->set_cooldown_ratio(status.cooldown_ratio);
	status_resource->set_cooldown_ticks_remaining((int64_t)status.cooldown_ticks_remaining);

	TypedArray<String> missing_upgrades;
	for (std::string const &upgrade : status.missing_upgrades) {
		missing_upgrades.append(String(upgrade.c_str()));
	}
	status_resource->set_missing_upgrades(missing_upgrades);

	TypedArray<String> other_explanations;
	for (std::string const &explanation : status.other_explanations) {
		other_explanations.append(String(explanation.c_str()));
	}
	status_resource->set_other_explanations(other_explanations);

	std::vector<std::pair<std::string, octopus::Fixed>> sorted_costs(status.resource_cost.begin(), status.resource_cost.end());
	std::sort(sorted_costs.begin(), sorted_costs.end(), [](auto const &a, auto const &b) { return a.first < b.first; });

	TypedArray<InputStatusResourceCost> resource_cost;
	for (auto const &[name, value] : sorted_costs) {
		Ref<InputStatusResourceCost> cost_resource(memnew(InputStatusResourceCost));
		cost_resource->set_resource(name.c_str());
		cost_resource->set_amount(value.to_double());
		resource_cost.append(cost_resource);
	}
	status_resource->set_resource_cost(resource_cost);

	return status_resource;
}

std::vector<flecs::entity> _collect_valid_entities(Ref<EntityGroup> group) {
	std::vector<flecs::entity> entities;
	entities.reserve(group->get_entities().size());
	for (flecs::entity const &entity : group->get_entities()) {
		if (entity.is_valid()) {
			entities.push_back(entity);
		}
	}
	return entities;
}

} // namespace

std::string InputStatusProxyNode::_build_query_key(char const *prefix, uint64_t group_id, String const &name) {
	return std::string(prefix) + ":" + std::to_string(group_id) + ":" + std::string(name.utf8().get_data());
}

Ref<InputStatusResource> InputStatusProxyNode::_duplicate_status_resource(Ref<InputStatusResource> const &resource) {
	if (!resource.is_valid()) {
		return Ref<InputStatusResource>();
	}

	Ref<InputStatusResource> copy(memnew(InputStatusResource));
	copy->set_ok(resource->get_ok());
	copy->set_entity_id(resource->get_entity_id());
	copy->set_cooldown_ratio(resource->get_cooldown_ratio());
	copy->set_cooldown_ticks_remaining(resource->get_cooldown_ticks_remaining());

	TypedArray<String> missing_upgrades;
	missing_upgrades.resize(resource->get_missing_upgrades().size());
	for (int i = 0; i < resource->get_missing_upgrades().size(); ++i) {
		missing_upgrades[i] = resource->get_missing_upgrades()[i];
	}
	copy->set_missing_upgrades(missing_upgrades);

	TypedArray<String> explanations;
	explanations.resize(resource->get_other_explanations().size());
	for (int i = 0; i < resource->get_other_explanations().size(); ++i) {
		explanations[i] = resource->get_other_explanations()[i];
	}
	copy->set_other_explanations(explanations);

	TypedArray<InputStatusResourceCost> costs;
	costs.resize(resource->get_resource_cost().size());
	for (int i = 0; i < resource->get_resource_cost().size(); ++i) {
		Ref<InputStatusResourceCost> src = resource->get_resource_cost()[i];
		Ref<InputStatusResourceCost> dst(memnew(InputStatusResourceCost));
		if (src.is_valid()) {
			dst->set_resource(src->get_resource());
			dst->set_amount(src->get_amount());
		}
		costs[i] = dst;
	}
	copy->set_resource_cost(costs);

	return copy;
}

void InputStatusProxyNode::_bind_methods() {
	BIND_NODE_PATH(InputStatusProxyNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &InputStatusProxyNode::setup);
	ClassDB::bind_method(D_METHOD("get_cast_status_from_group", "group", "cast_name"), &InputStatusProxyNode::get_cast_status_from_group);
	ClassDB::bind_method(D_METHOD("get_production_status_from_group", "group", "production_name"), &InputStatusProxyNode::get_production_status_from_group);
	ClassDB::bind_method(D_METHOD("drop_cast_query", "group", "cast_name"), &InputStatusProxyNode::drop_cast_query);
	ClassDB::bind_method(D_METHOD("drop_production_query", "group", "production_name"), &InputStatusProxyNode::drop_production_query);
	ClassDB::bind_method(D_METHOD("drop_all_cast_queries"), &InputStatusProxyNode::drop_all_cast_queries);
	ClassDB::bind_method(D_METHOD("drop_all_production_queries"), &InputStatusProxyNode::drop_all_production_queries);
}

void InputStatusProxyNode::setup() {
	if (_setup_done || !_game_node) {
		return;
	}

	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, ecs](flecs::iter &) {
			std::lock_guard<std::mutex> lock(_mutex);
			flecs::world world = ecs;
			_refresh_cast_queries(world);
			_refresh_production_queries(world);
		});

	_setup_done = true;
}

Ref<InputStatusResource> InputStatusProxyNode::get_cast_status_from_group(Ref<EntityGroup> group, String const &cast_name) {
	if (!_game_node) {
		return _make_status_with_reason("MISSING_GAME_NODE");
	}
	if (!group.is_valid()) {
		return _make_status_with_reason("NO_VALID_GROUP");
	}
	if (cast_name.is_empty()) {
		return _make_status_with_reason("EMPTY_CAST_NAME");
	}

	std::lock_guard<std::mutex> lock(_mutex);
	std::string key = _build_query_key("cast", group->get_instance_id(), cast_name);
	CastQuery &query = _cast_queries[key];
	query.group = group;
	query.cast_name = cast_name.utf8().get_data();
	if (!query.status.is_valid()) {
		query.status = _make_status_with_reason("NOT_READY");
	}
	return _duplicate_status_resource(query.status);
}

Ref<InputStatusResource> InputStatusProxyNode::get_production_status_from_group(Ref<EntityGroup> group, String const &production_name) {
	if (!_game_node) {
		return _make_status_with_reason("MISSING_GAME_NODE");
	}
	if (!group.is_valid()) {
		return _make_status_with_reason("NO_VALID_GROUP");
	}
	if (production_name.is_empty()) {
		return _make_status_with_reason("EMPTY_PRODUCTION_NAME");
	}

	std::lock_guard<std::mutex> lock(_mutex);
	std::string key = _build_query_key("prod", group->get_instance_id(), production_name);
	ProductionQuery &query = _production_queries[key];
	query.group = group;
	query.production_name = production_name.utf8().get_data();
	if (!query.status.is_valid()) {
		query.status = _make_status_with_reason("NOT_READY");
	}
	return _duplicate_status_resource(query.status);
}

void InputStatusProxyNode::drop_cast_query(Ref<EntityGroup> group, String const &cast_name) {
	if (!group.is_valid() || cast_name.is_empty()) {
		return;
	}

	std::lock_guard<std::mutex> lock(_mutex);
	std::string key = _build_query_key("cast", group->get_instance_id(), cast_name);
	_cast_queries.erase(key);
}

void InputStatusProxyNode::drop_production_query(Ref<EntityGroup> group, String const &production_name) {
	if (!group.is_valid() || production_name.is_empty()) {
		return;
	}

	std::lock_guard<std::mutex> lock(_mutex);
	std::string key = _build_query_key("prod", group->get_instance_id(), production_name);
	_production_queries.erase(key);
}

void InputStatusProxyNode::drop_all_cast_queries() {
	std::lock_guard<std::mutex> lock(_mutex);
	_cast_queries.clear();
}

void InputStatusProxyNode::drop_all_production_queries() {
	std::lock_guard<std::mutex> lock(_mutex);
	_production_queries.clear();
}

void InputStatusProxyNode::_refresh_cast_queries(flecs::world &ecs) {
	auto const *ability_library = ecs.try_get<octopus::AbilityTemplateLibrary<custom_step_manager>>();
	if (!ability_library) {
		for (auto &[_, query] : _cast_queries) {
			query.status = _make_status_with_reason("MISSING_ABILITY_LIBRARY");
		}
		return;
	}

	for (auto &[_, query] : _cast_queries) {
		if (!query.group.is_valid()) {
			query.status = _make_status_with_reason("NO_VALID_GROUP");
			continue;
		}

		std::vector<flecs::entity> entities = _collect_valid_entities(query.group);
		if (entities.empty()) {
			query.status = _make_status_with_reason("NO_VALID_CANDIDATE");
			continue;
		}

		octopus::InputCast input;
		input.candidates = entities;
		input.cast_command = octopus::CastCommand {query.cast_name, flecs::entity(), octopus::Vector()};
		query.status = _convert_input_status(octopus::get_input_status(ecs, *ability_library, input));
	}
}

void InputStatusProxyNode::_refresh_production_queries(flecs::world &ecs) {
	auto const *production_library = ecs.try_get<octopus::ProductionTemplateLibrary<custom_step_manager>>();
	if (!production_library) {
		for (auto &[_, query] : _production_queries) {
			query.status = _make_status_with_reason("MISSING_PRODUCTION_LIBRARY");
		}
		return;
	}

	for (auto &[_, query] : _production_queries) {
		if (!query.group.is_valid()) {
			query.status = _make_status_with_reason("NO_VALID_GROUP");
			continue;
		}

		std::vector<flecs::entity> entities = _collect_valid_entities(query.group);
		if (entities.empty()) {
			query.status = _make_status_with_reason("NO_VALID_CANDIDATE");
			continue;
		}

		octopus::InputProduction input;
		input.candidates = entities;
		input.production = query.production_name;
		query.status = _convert_input_status(octopus::get_input_status(ecs, *production_library, input));
	}
}

void InputStatusProxyNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if (_game_node) {
		_game_node->connect("init_done", callable_mp(this, &InputStatusProxyNode::setup));
	}
}

void InputStatusProxyNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
		} break;
	}
}

} // namespace godot
