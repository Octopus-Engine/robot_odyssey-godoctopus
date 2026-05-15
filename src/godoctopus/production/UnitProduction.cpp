#include "UnitProduction.h"

#include "octopus/world/step/EntityCreationStep.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "octopus/components/basic/rally_point/RallyPoint.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"

namespace godot {

UnitProductionTemplate::UnitProductionTemplate(Ref<UnitPrefab> const &prefab, ResourceNodeEventBus* event_bus_p) : event_bus(event_bus_p) {
	_name = prefab->get_prefab_name().utf8().get_data();
	duration_in_ticks = prefab->get_production_duration() * TICK_RATE;
	// Requirements handled in godot for now (temporary!)
	//_resource_consumption["basic"] = prefab->get_cost_basic_x10() / 10.;
	//_resource_consumption["advanced"] = prefab->get_cost_advanced_x10() / 10.;
}

/// @brief This is used to handle resource consumption and restoration
std::unordered_map<std::string, octopus::Fixed> UnitProductionTemplate::resource_consumption() const {
	return _resource_consumption;
}

/// @brief This is called when the production is done
/// this must materialize the production into the world
/// @note example : spawn a unit
void UnitProductionTemplate::produce(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
	octopus::EntityCreationStep step_l;

	step_l.set_up_function = [&, producer_p](flecs::entity e, flecs::world const &world_p) {
		custom_queue queue;

		// rally point
		octopus::RallyPoint const * rally_point = producer_p.try_get<octopus::RallyPoint>();
		if(rally_point && rally_point->enabled)
		{
			octopus::MoveCommand move;
			move.target = rally_point->target;
			move.extra_tolerance = rally_point->tolerance;
			queue._queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {move});
		}

		octopus::Position pos_l;
		pos_l.pos = producer_p.try_get<octopus::Position>()->pos;
		if (producer_p.try_get<octopus::ProductionQueue>()) {
			pos_l.pos += producer_p.try_get<octopus::ProductionQueue>()->spawn_point;
		} else {
			pos_l.pos.y += 5; // to avoid collision with producer
		}

		e.set<octopus::Position>(pos_l)
			.is_a(ecs.prefab(_name.c_str()))
			.set<custom_queue>(queue);

		// Copy player ownership from producer
		octopus::Team const * team = producer_p.try_get<octopus::Team>();
		if(team) {
			e.set<octopus::Team>(*team);
		}

		octopus::PlayerAppartenance const * player_app = producer_p.try_get<octopus::PlayerAppartenance>();
		if(player_app) {
			e.set<octopus::PlayerAppartenance>(*player_app);
		}
	};

	ecs.try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step_l);
}

/// @brief This is called when the production is enqueued
/// @note example : consume resources
void UnitProductionTemplate::enqueue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
	if (!event_bus) {
		return;
	}
	auto pos = producer_p.try_get<octopus::Position>();
	Vector3 godot_pos(
		real_t(pos->pos.x.to_double()) * WORLD_SCALE,
		0.f,
		real_t(pos->pos.y.to_double()) * WORLD_SCALE);
	for (auto const &[resource_name, amount] : resource_consumption()) {
		event_bus->notify_resource_consumed(
			godot_pos,
			String(resource_name.c_str()),
			amount.to_double(),
			octopus::get_player_from_appartenance(producer_p, ecs).get<octopus::PlayerInfo>().idx
		);
	}
}

/// @brief This is called when the production is dequeued
/// @note example : restore resources
void UnitProductionTemplate::dequeue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
	if (!event_bus) {
		return;
	}
	auto pos = producer_p.try_get<octopus::Position>();
	Vector3 godot_pos(
		real_t(pos->pos.x.to_double()) * WORLD_SCALE,
		0.f,
		real_t(pos->pos.y.to_double()) * WORLD_SCALE);
	for (auto const &[resource_name, amount] : resource_consumption()) {
		event_bus->notify_resource_produced(
			godot_pos,
			String(resource_name.c_str()),
			amount.to_double(),
			octopus::get_player_from_appartenance(producer_p, ecs).get<octopus::PlayerInfo>().idx
		);
	}
}

/// @brief return a unique name for this production
std::string UnitProductionTemplate::name() const {
	return _name;
}

/// @brief This is the duration (in steps) during which
/// the production will be the current element of the queue
/// before being produced.
int64_t UnitProductionTemplate::duration() const {
	return duration_in_ticks;
}

} // namespace godot