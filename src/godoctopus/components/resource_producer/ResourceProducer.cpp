#include "ResourceProducer.h"

#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"
#include "octopus/systems/phases/Phases.hh"

void declare_resource_producer_system(flecs::world &ecs, custom_step_manager &step_manager)
{
	ecs.system<ResourceProducer, octopus::PlayerAppartenance const>()
		.kind(ecs.entity(PostUpdatePhase))
		.each([&step_manager](flecs::entity e, ResourceProducer &producer, octopus::PlayerAppartenance const &) {
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
		});
}
