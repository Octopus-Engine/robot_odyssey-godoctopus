#pragma once

#include "flecs.h"

#include "octopus/world/ability/AbilityTemplateLibrary.hh"
#include "octopus/components/step/BuffComponentStep.hh"
#include "octopus/components/advanced/buff/BuffSystem.hh"

#include "octopus_types.h"

// EarbotSteam
struct EarbotSteamBuff {
	int32_t reload_diminisher = TICK_RATE/2;
};

struct EarbotSteam : octopus::AbilityTemplate<custom_step_manager> {
	virtual octopus::UpgradeRequirement get_requirements() const {
		return {};
	}
	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const {
		return {};
	}
	virtual void cast(flecs::entity caster, octopus::Vector, flecs::entity, flecs::world const &ecs, custom_step_manager &manager_p) const {
		using namespace octopus;
		AddBuffComponentStep<EarbotSteamBuff> step;
		step.start = get_time_stamp(ecs);
		step.duration = 15*TICK_RATE;
		manager_p.get_last_component_layer().back().add_step(caster, std::move(step));
	}

	static std::string NAME() { return "earbot_steam"; }
	virtual std::string name() const { return EarbotSteam::NAME(); }
	virtual int64_t windup() const { return 2; }
	virtual int64_t reload() const { return TICK_RATE*120; }
	virtual bool need_point_target() const { return false; }
	virtual bool need_entity_target() const { return false; }
	virtual octopus::Fixed range() const { return 0; }
};

void declare_earbot_steam_ability(flecs::world &ecs, octopus::DefaultStepContext<custom_variant>& step_context);
