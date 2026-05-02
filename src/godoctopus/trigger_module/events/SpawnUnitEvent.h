#pragma once

#include "flecs.h"

#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "godoctopus/components/types/Types.h"

// Trigger a new event to spawn a new entity
template<typename ComponentEvent>
struct SpawnCopyEntityEvent {
	static void apply(flecs::entity source, int32_t) {
		if (source.try_get<ComponentEvent>()) {
			return;
		}
		auto ecs = source.world();

		octopus::Position pos = source.get<octopus::Position>();
		octopus::Team team = source.get<octopus::Team>();
		std::string type = source.get<PrefabType>().name;

		octopus::EntityCreationStep step_l;
		step_l.set_up_function = [pos, team, type](flecs::entity new_ent, flecs::world const &world_p) {
			new_ent.set<octopus::Position>(pos)
				.is_a(world_p.prefab(type.c_str()))
				.set<octopus::Team>(team)
				.set<octopus::PlayerAppartenance>({(uint32_t)team.team})
				.add<ComponentEvent>()
			;
		};

		ecs.try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step_l);

		// source.world().event<SpawnCopyEntityEvent<Component>>()
		// 		.id<ComponentEvent>()
		// 		.entity(source)
		// 		.emit();
	}
};

// void declare_spawn_copy_event_system(flecs::world &ecs, custom_step_manager &manager) {


// 	ecs.observer<Rune const, typename Trigger::component const>()
// 		.template event<Trigger>()
// 		.each([&ctx](flecs::entity e, Rune const &rune, typename Trigger::component const &) {
// 			octopus::Position const *pos = e.try_get<octopus::Position>();
// 			octopus::Team const *team = e.try_get<octopus::Team>();
// 			if(pos && team && Condition::check(e))
// 			{
// 				Event::apply(e, pos->pos, team->team, ctx, rune.level);
// 			}
// 		});

// 	octopus::EntityCreationStep step_l;
// 	step_l.set_up_function = [ecs, pos_l, type, spawn_action](flecs::entity new_ent, flecs::world const &world_p) {
// 		new_ent.set<octopus::Position>(pos_l)
// 			.is_a(ecs.prefab(type.c_str()))
// 			.set<octopus::Team>({(uint16_t)spawn_action.team})
// 			.set<octopus::PlayerAppartenance>({(uint32_t)spawn_action.team})
// 			.set<custom_queue>(queue_l)
// 		;
// 	};
// }