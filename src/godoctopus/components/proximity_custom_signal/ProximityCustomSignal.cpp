#include "ProximityCustomSignal.h"

#include <functional>
#include <vector>

#include "octopus/systems/phases/Phases.hh"
#include "octopus/utils/aabb/aabb_tree.hh"

#include "godoctopus/custom_signal/CustomSignalNode.h"

void declare_proximity_custom_signal_system(flecs::world &ecs, octopus::PositionContext &pos_context)
{
	flecs::entity custom_signal_entity = ecs.entity(godot::CustomSignalNode::NAME());

	ecs.system<ProximityCustomSignal, octopus::Position const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.each([&pos_context, custom_signal_entity, ecs](flecs::entity e, ProximityCustomSignal &checker, octopus::Position const &pos) {
			if (checker.refresh_tick > 0 && octopus::get_time_stamp(ecs) % checker.refresh_tick != 0) {
				return;
			}
			if (checker.tree_idx < 0 || size_t(checker.tree_idx) >= pos_context.trees.size()) {
				print_line("ProximityCustomSignal has invalid tree_idx!");
				e.remove<ProximityCustomSignal>();
				return;
			}

			bool found = false;
			std::function<bool(int32_t, flecs::entity)> func_l = [&found, e](int32_t, flecs::entity other) -> bool {
				found = other != e;
				return false;
			};
			tree_circle_query(pos_context.trees[checker.tree_idx], pos.pos, checker.range, func_l);

			if (!found) {
				return;
			}

			godot::CustomSignalEvent custom_signal_event {
				checker.payload,
				{
					float(pos.pos.x.to_double()),
					float(pos.pos.y.to_double())
				}
			};
			custom_signal_entity.emit<godot::CustomSignalEvent>(custom_signal_event);
			e.remove<ProximityCustomSignal>();
		});
}
