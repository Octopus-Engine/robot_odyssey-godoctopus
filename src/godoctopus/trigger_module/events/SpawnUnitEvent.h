#pragma once

#include "flecs.h"

#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
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
		octopus::HitPointMax hpMax = source.get<octopus::HitPointMax>();
		std::string type = source.get<PrefabType>().name;

		octopus::EntityCreationStep step_l;
		step_l.set_up_function = [pos, team, type, hpMax](flecs::entity new_ent, flecs::world const &world_p) {
			new_ent.set<octopus::Position>(pos)
				.is_a(world_p.prefab(type.c_str()))
				.set<octopus::HitPoint>({hpMax.qty/2})
				.set<octopus::Team>(team)
				.set<octopus::PlayerAppartenance>({(uint32_t)team.team})
				.add<ComponentEvent>()
			;
		};

		ecs.try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step_l);
	}
};
