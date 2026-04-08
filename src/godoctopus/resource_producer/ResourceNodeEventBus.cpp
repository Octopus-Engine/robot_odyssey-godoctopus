#include "ResourceNodeEventBus.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"
#include "octopus/systems/phases/Phases.hh"

namespace godot {

void ResourceNodeEventBus::_bind_methods() {
	BIND_NODE_PATH(ResourceNodeEventBus, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &ResourceNodeEventBus::setup);
	ClassDB::bind_method(D_METHOD("notify_resource_consumed", "position", "resource_name", "amount", "player"),
		&ResourceNodeEventBus::notify_resource_consumed);

	ADD_SIGNAL(MethodInfo("resource_produced",
		PropertyInfo(Variant::VECTOR3, "position"),
		PropertyInfo(Variant::STRING, "resource_name"),
		PropertyInfo(Variant::FLOAT, "amount"),
		PropertyInfo(Variant::INT, "player")));

	ADD_SIGNAL(MethodInfo("resource_consumed",
		PropertyInfo(Variant::VECTOR3, "position"),
		PropertyInfo(Variant::STRING, "resource_name"),
		PropertyInfo(Variant::FLOAT, "amount"),
		PropertyInfo(Variant::INT, "player")));
}

void ResourceNodeEventBus::setup() {
	if (!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;
	custom_step_manager &step_manager = _game_node->get_step_context().step_manager;

	ecs.system<ResourceProducer, octopus::PlayerAppartenance const, octopus::Position const>()
		.kind(ecs.entity(PostUpdatePhase))
		.each([this, &step_manager](flecs::entity e, ResourceProducer &producer,
				octopus::PlayerAppartenance const &appartenance,
				octopus::Position const &pos) {
			++producer.ticks_since_last;
			if (producer.ticks_since_last < producer.interval) {
				return;
			}
			producer.ticks_since_last = 0;

			flecs::entity player = octopus::get_player_from_appartenance(e, e.world());
			if (!player.is_valid()) {
				return;
			}
			step_manager.get_last_layer().back().template get<octopus::ResourceStockStep>().add_step(
				player, {producer.amount, producer.resource_name});

			ResourceProducedEvent event;
			event.position = pos.pos;
			event.resource = producer.resource_name;
			event.amount = producer.amount.to_double();
			event.player_idx = appartenance.idx;

			std::lock_guard<std::mutex> lock(_mutex);
			_pending_events.push_back(std::move(event));
		});
}

void ResourceNodeEventBus::notify_resource_consumed(Vector3 const &position, String const &resource_name, double amount, int player) {
	call_deferred("emit_signal", "resource_consumed",
		position,
		resource_name,
		amount,
		player);
}

void ResourceNodeEventBus::_process(double delta) {
	std::lock_guard<std::mutex> lock(_mutex);
	for (ResourceProducedEvent const &event : _pending_events) {
		Vector3 godot_pos(
			real_t(event.position.x.to_double()) * WORLD_SCALE,
			0.f,
			real_t(event.position.y.to_double()) * WORLD_SCALE);
		call_deferred("emit_signal", "resource_produced",
			godot_pos,
			String(event.resource.c_str()),
			event.amount,
			int(event.player_idx));
	}
	_pending_events.clear();
}

void ResourceNodeEventBus::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if (_game_node) {
		_game_node->connect("init_done", callable_mp(this, &ResourceNodeEventBus::setup));
	}
}

void ResourceNodeEventBus::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			set_process(true);
			init_nodes();
		} break;
	}
}

} // namespace godot
