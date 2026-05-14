#pragma once

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/production/ProductionTemplateLibrary.hh"

#include "octopus_types.h"

struct UpgradeDone {};

template<typename UpgradeComponent>
struct EntityUpgradeProduction : octopus::ProductionTemplate<custom_step_manager>
{
	EntityUpgradeProduction(std::string const &prod_name_p, int64_t duration_p,
		std::unordered_map<std::string, octopus::Fixed> const &costs_p,
		std::vector<std::string> const &requirements_p = {})
		: prod_name(prod_name_p), duration_prod(duration_p), costs(costs_p), requirements(requirements_p) {}

	std::string const prod_name;
	int64_t const duration_prod = 0;
	std::unordered_map<std::string, octopus::Fixed> const costs;
	std::vector<std::string> const requirements;

	virtual octopus::UpgradeRequirement get_requirements() const {
		octopus::UpgradeRequirement up_requirement;
		for(std::string const &req : requirements)
		{
			up_requirement.upgrades[req] = 1;
		}
		return up_requirement;
	}
    virtual bool can_produce(flecs::entity producer_p, flecs::world const &ecs) const {
		return !producer_p.has<UpgradeComponent>();
	}

	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const { return costs; }
	virtual void produce(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		apply_production(producer_p, ecs, manager_p);

		// ecs.event<UpgradeDone>()
		// 	.id<octopus::PlayerInfo>()
		// 	.entity(player)
		// 	.emit();
	}
	virtual void apply_production(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {}
	virtual void enqueue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		manager_p.get_last_layer().back().template get<octopus::AddComponentStep>().add_step(producer_p, {UpgradeComponent()});

		// ecs.event<UpgradeDone>()
		// 	.id<octopus::PlayerInfo>()
		// 	.entity(player)
		// 	.emit();
	}
	virtual void dequeue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		manager_p.get_last_layer().back().template get<octopus::RemoveComponentStep>().add_step(producer_p, {});

		// ecs.event<UpgradeDone>()
		// 	.id<octopus::PlayerInfo>()
		// 	.entity(player)
		// 	.emit();
	}
	virtual std::string name() const { return prod_name+"_upgrade"; }
	virtual int64_t duration() const { return duration_prod;}
};
