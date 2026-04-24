#pragma once

#include <cmath>
#include "flecs.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/world/ability/AbilityTemplateLibrary.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "octopus/utils/log/Logger.hh"

#include "octopus_types.h"

namespace godot {
class GameNode;
}

struct CircularSpawnAbility : octopus::AbilityTemplate<custom_step_manager> {

	virtual octopus::UpgradeRequirement get_requirements() const { return {}; }
	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const { return {}; }

	// Pure virtual methods for subclasses to customize
	/// @brief Get the radius of the spawn circle
	virtual octopus::Fixed get_spawn_radius() const = 0;

	/// @brief Get the number of entities to spawn in the circle
	virtual uint32_t get_spawn_count() const = 0;

	/// @brief Get the prefab name for spawned entities
	virtual std::string get_prefab_name() const = 0;

	// Allow subclasses to add custom castability checks.
	// Return non-empty string to indicate an error, empty string if check passes.
	virtual std::string get_additional_castability_errors(flecs::entity, flecs::world const &) const {
		return "";
	}

	virtual std::string is_castable(flecs::entity caster, flecs::world const &ecs) const override {
		return get_additional_castability_errors(caster, ecs);
	}

	virtual void cast(flecs::entity caster, octopus::Vector, flecs::entity, flecs::world const &ecs, custom_step_manager &) const {
		octopus::Logger::getDebug() << "CircularSpawn :: Entering" << std::endl;

		octopus::Position const *pos = caster.try_get<octopus::Position>();
		if (!pos) {
			return;
		}

		octopus::Fixed radius = get_spawn_radius();
		uint32_t count = get_spawn_count();
		std::string prefab_name = get_prefab_name();

		if (count == 0) {
			return;
		}

		octopus::Logger::getDebug() << "CircularSpawn :: ability cast with radius=" << radius 
									 << ", count=" << count << ", prefab=" << prefab_name << std::endl;

		octopus::PlayerAppartenance const *appartenance = caster.try_get<octopus::PlayerAppartenance>();
		uint32_t player_idx = appartenance ? appartenance->idx : 0;
		octopus::Team const *team = caster.try_get<octopus::Team>();
		uint16_t player_team = team ? team->team : 0;

		octopus::Position spawn_pos = *pos;

		// Calculate spawn positions in a circle
		// angle_step = 2π / count, where we approximate 2π as a Fixed value
		// For Fixed arithmetic: 2π ≈ 6.283185 * 10000 / 10000
		const octopus::Fixed two_pi = octopus::Fixed(62832, true); // 2π as fixed point

		for (uint32_t i = 0; i < count; ++i) {
			// Calculate angle for this spawn point: π / count + 2π * i / count
			octopus::Fixed angle = two_pi / (2 * octopus::Fixed(count)) + two_pi * octopus::Fixed(i) / octopus::Fixed(count);

			// Convert to double for trigonometry (convert back to Fixed after)
			double angle_double = angle.to_double();
			double cos_angle = std::cos(angle_double);
			double sin_angle = std::sin(angle_double);

			// Calculate offset: (radius * cos(angle), radius * sin(angle))
			double radius_double = radius.to_double();
			octopus::Fixed offset_x = octopus::Fixed(radius_double * cos_angle);
			octopus::Fixed offset_y = octopus::Fixed(radius_double * sin_angle);

			// Calculate final spawn position
			octopus::Vector final_pos = spawn_pos.pos;
			final_pos.x += offset_x;
			final_pos.y += offset_y;

			// Create entity spawn step
			octopus::EntityCreationStep step;
			step.set_up_function = [final_pos, player_idx, player_team, prefab_name](flecs::entity new_ent, flecs::world const &world_p) {
				auto e = new_ent
					.set<octopus::Position>(octopus::Position{{final_pos.x, final_pos.y}, {0, 0}, {}})
					.set<octopus::Team>({player_team})
					.set<octopus::PlayerAppartenance>({player_idx});
				if (!prefab_name.empty()) {
					e.is_a(world_p.prefab(prefab_name.c_str()));
				}
			};

			// StepEntityManager is always mutably accessible — cast away const on the world wrapper
			const_cast<flecs::world &>(ecs).try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step);
		}
	}

	static std::string NAME() { return "circular_spawn"; }
	virtual std::string name() const { return CircularSpawnAbility::NAME(); }
	virtual int64_t windup() const { return 1; }
	virtual int64_t reload() const { return 0; }
	virtual bool need_point_target() const { return false; }
	virtual bool need_entity_target() const { return false; }
	virtual octopus::Fixed range() const { return octopus::Fixed(0); }
};

void declare_circular_spawn_ability(flecs::world &ecs, godot::GameNode &game);
