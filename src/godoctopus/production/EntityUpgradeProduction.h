#pragma once

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/production/ProductionTemplateLibrary.hh"
#include "godoctopus/event_bus/ProductionEvent.h"

#include "octopus_types.h"

template<typename UpgradeComponent>>
struct UpgradeState {
	bool active = false;
};

/// @brief Producer entity must have the component UpgradeState<UpgradeComponent> for the production to be producable.
/// On enqueue, the production will set UpgradeState<UpgradeComponent>::active to true and add UpgradeComponent to the
/// producer entity. On dequeue, it will set active to false.
/// @tparam UpgradeComponent
template<typename UpgradeComponent>
struct EntityUpgradeProduction : octopus::ProductionTemplate<custom_step_manager> {
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
		return producer_p.has<UpgradeState<UpgradeComponent>>()
			&& producer_p.get<UpgradeState<UpgradeComponent>>().active == false;
	}

	virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const { return costs; }
	virtual void produce(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		apply_production(producer_p, ecs, manager_p);

		ecs.entity(PRODUCTION_NODE_EVENT_BUS).emit<ProductionDone>(producer_p, {prod_name, (int)player.get<octopus::PlayerInfo>().idx});

		// ecs.event<UpgradeDone>()
		// 	.id<octopus::PlayerInfo>()
		// 	.entity(player)
		// 	.emit();
	}
	virtual void apply_production(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {}
	virtual void enqueue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		producer_p.get_mut<UpgradeState<UpgradeComponent>>().active = true;
		manager_p.get_last_layer().back().template get<octopus::AddComponentStep>().add_step(producer_p, {UpgradeComponent()});

		// ecs.event<UpgradeDone>()
		// 	.id<octopus::PlayerInfo>()
		// 	.entity(player)
		// 	.emit();
	}
	virtual void dequeue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const {
		producer_p.get_mut<UpgradeState<UpgradeComponent>>().active = false;
		manager_p.get_last_layer().back().template get<octopus::RemoveComponentStep>().add_step(producer_p, {});

		// ecs.event<UpgradeDone>()
		// 	.id<octopus::PlayerInfo>()
		// 	.entity(player)
		// 	.emit();
	}
	virtual std::string name() const { return prod_name+"_upgrade"; }
	virtual int64_t duration() const { return duration_prod;}

	static void declare_component(flecs::world &ecs) {
		ecs.component<UpgradeState<UpgradeComponent>>()
			.member<bool>("active");
	}
};
