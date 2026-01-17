#pragma once

#include "flecs.h"

#include "octopus/world/ability/AbilityTemplateLibrary.hh"
#include "octopus/components/step/BuffComponentStep.hh"
#include "octopus/components/advanced/buff/BuffSystem.hh"
#include "octopus/world/player/PlayerInfo.hh"

#include "octopus_types.h"

namespace godot {
class GameNode;
}

// ArmorbotAbility
struct ArmorbotBuff {
	octopus::Fixed buff = 6;
};

struct ArmorbotAbility : octopus::AbilityTemplate<custom_step_manager> {
	ArmorbotAbility(octopus::PositionContext const &pos_ctx) : ctx(pos_ctx) {}

	virtual octopus::UpgradeRequirement get_requirements() const {
		return {};
	}
	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const {
		return {};
	}

    virtual void cast(flecs::entity caster, octopus::Vector target_point, flecs::entity target_entity, flecs::world const &ecs, custom_step_manager &manager) const {
		using namespace octopus;
		flecs::entity player = octopus::get_player_from_appartenance(caster, ecs);
		uint32_t team = player.try_get<octopus::PlayerInfo>()->team;

		AddBuffComponentStep<ArmorbotBuff> step;
		step.start = get_time_stamp(ecs);
		step.duration = 15*TICK_RATE;

		std::function<bool(int32_t, flecs::entity)> func_l = [&manager, &caster, team, &step](int32_t idx_l, flecs::entity e) -> bool {
			if(e.try_get<octopus::Team>()
			&& e.try_get<octopus::Team>()->team == team)
			{
				manager.get_last_component_layer().back().add_step(caster, step);
			}
			return true;
		};

		tree_circle_query(ctx.trees[0], target_point, aoe_range(), func_l);
	}

	static std::string NAME() { return "armorbot_ability"; }
	virtual std::string name() const { return ArmorbotAbility::NAME(); }
	virtual int64_t windup() const { return 2; }
	virtual int64_t reload() const { return TICK_RATE*120; }
	virtual bool need_point_target() const { return true; }
	virtual bool need_entity_target() const { return false; }
	virtual octopus::Fixed range() const { return 7; }

	// godoctopus specifics
	virtual octopus::Fixed aoe_range() const { return 4; }
private:
	octopus::PositionContext const &ctx;
};

void declare_armorbot_ability(flecs::world &ecs, godot::GameNode &game);
