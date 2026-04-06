#include "ProximitySensor.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/utils/aabb/aabb_tree.hh"
#include "octopus/systems/phases/Phases.hh"

void declare_proximity_sensor_system(flecs::world &ecs, octopus::PositionContext &pos_context)
{
	ecs.system<ProximitySensor, octopus::Position const>()
		.kind(ecs.entity(PostUpdatePhase))
		.each([&pos_context](flecs::entity e, ProximitySensor &sensor, octopus::Position const &pos) {
			++sensor.ticks_since_check;
			if (sensor.ticks_since_check < sensor.refresh_rate) {
				return;
			}
			sensor.ticks_since_check = 0;

			bool found = false;

			size_t tree_idx = 0;
			size_t team_idx = 0;
			octopus::Team const *team = e.try_get<octopus::Team>();
			if (team && team->team < pos_context.trees_team_hp.size()) {
				team_idx = team->team;
			}
			// With only two teams, the ally tree is the enemy tree of the other team.
			if (sensor.check_allies) {
				tree_idx = pos_context.trees_team_hp[(1 + team_idx) % 2];
			} else {
				tree_idx = pos_context.trees_team_hp[team_idx];
			}

			std::function<bool(int32_t, flecs::entity)> func = [&found, e](int32_t, flecs::entity other) -> bool {
				if (other != e) {
					found = true;
					return false;
				}
				return true;
			};
			tree_circle_query(pos_context.trees[tree_idx], pos.pos, sensor.range, func);
			if (found) {
				sensor.activated = true;
			}
		});
}
