#pragma once

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/production/ProductionTemplateLibrary.hh"
#include "godoctopus/event_bus/ProductionEvent.h"

#include "octopus_types.h"


struct UpgradeUpdate {};

struct PlayerUpgradeProduction : octopus::ProductionTemplate<custom_step_manager> {
	PlayerUpgradeProduction(std::string const &prod_name_p, int64_t duration_p,
		std::unordered_map<std::string, octopus::Fixed> const &costs_p = {},
		std::vector<std::string> const &requirements_p = {})
		: prod_name(prod_name_p), duration_prod(duration_p), costs(costs_p), requirements(requirements_p) {}

	std::string const prod_name;
	int64_t const duration_prod = 0;
	std::unordered_map<std::string, octopus::Fixed> const costs;
	std::vector<std::string> const requirements;

	virtual octopus::UpgradeRequirement get_requirements() const {
		octopus::UpgradeRequirement up_requirement;
		for(std::string const &req : requirements) {
			up_requirement.upgrades[req] = 1;
		}
		up_requirement.upgrades["INTERNAL_UPGRADE_"+prod_name] = 1;
		return up_requirement;
	}
	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const { return costs; }
	virtual void produce(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		flecs::entity player = octopus::get_player_from_appartenance(producer_p, ecs);
		manager_p.get_last_layer().back().template get<octopus::PlayerUpgradeStep>().add_step(player, {prod_name});
		manager_p.get_last_layer().back().template get<octopus::PlayerUpgradeStep>().add_step(player, {"INTERNAL_UPGRADE_"+prod_name, -1});

		apply_production(producer_p, ecs, manager_p);

		ecs.entity(PRODUCTION_NODE_EVENT_BUS).emit<ProductionDone>(producer_p, {prod_name, (int)player.get<octopus::PlayerInfo>().idx});
	}
	virtual void apply_production(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {}
	virtual void enqueue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		flecs::entity player = octopus::get_player_from_appartenance(producer_p, ecs);
		manager_p.get_last_layer().back().template get<octopus::PlayerUpgradeStep>().add_step(player, {"INTERNAL_UPGRADE_"+prod_name, -1});

		ecs.event<UpgradeUpdate>()
			.id<octopus::PlayerInfo>()
			.entity(player)
			.emit();
	}
	virtual void dequeue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		flecs::entity player = octopus::get_player_from_appartenance(producer_p, ecs);
		manager_p.get_last_layer().back().template get<octopus::PlayerUpgradeStep>().add_step(player, {"INTERNAL_UPGRADE_"+prod_name, 1});

		ecs.event<UpgradeUpdate>()
			.id<octopus::PlayerInfo>()
			.entity(player)
			.emit();
	}
	virtual std::string name() const { return prod_name; }
	virtual int64_t duration() const { return duration_prod;}
};
