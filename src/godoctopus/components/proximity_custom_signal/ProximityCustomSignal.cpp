#include "ProximityCustomSignal.h"

#include <functional>
#include <vector>

#include "octopus/systems/phases/Phases.hh"
#include "octopus/utils/aabb/aabb_tree.hh"

#include "godoctopus/custom_signal/CustomSignalNode.h"

void declare_proximity_custom_signal_system(flecs::world &ecs, octopus::PositionContext &pos_context)
{
	flecs::entity custom_signal_entity = ecs.entity(godot::CustomSignalNode::NAME());

	ecs.system<ProximityCustomSignal>()
		.kind(ecs.entity(DisplaySyncPhase))
		.each([&pos_context, custom_signal_entity](flecs::entity e, ProximityCustomSignal &checker) {
			if (checker.tree_idx < 0 || size_t(checker.tree_idx) >= pos_context.trees.size()) {
				print_line("ProximityCustomSignal has invalid tree_idx!");
				e.remove<ProximityCustomSignal>();
				return;
			}

			bool found = false;
			std::function<bool(int32_t, flecs::entity)> func_l = [&found](int32_t, flecs::entity) -> bool {
				found = true;
				return false;
			};
			tree_circle_query(pos_context.trees[checker.tree_idx], checker.pos, checker.range, func_l);

			if (!found) {
				return;
			}

			godot::CustomSignalEvent custom_signal_event {
				checker.payload,
				{
					float(checker.pos.x.to_double()),
					float(checker.pos.y.to_double())
				}
			};
			custom_signal_entity.emit<godot::CustomSignalEvent>(custom_signal_event);
			e.remove<ProximityCustomSignal>();
		});
}
