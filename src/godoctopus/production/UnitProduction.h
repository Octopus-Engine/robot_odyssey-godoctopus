#pragma once

#include "octopus/world/production/ProductionTemplate.hh"
#include "octopus/world/production/ProductionTemplateLibrary.hh"

#include "godoctopus/game/prefabs/UnitPrefab.h"
#include "godoctopus/resource_producer/ResourceNodeEventBus.h"

#include "octopus_types.h"

namespace godot {

/// @brief This class represent a template for a production
/// entity, it can be a unit, an upgrade, or event a building
struct UnitProductionTemplate : public octopus::ProductionTemplate<custom_step_manager> {
	UnitProductionTemplate(Ref<UnitPrefab> const &prefab, ResourceNodeEventBus* event_bus_p);

    // Requirements handled in godot for now (temporary!)
    /// @brief This is used to handle resource consumption and restoration
    virtual std::unordered_map<std::string, octopus::Fixed> resource_consumption() const override;
    /// @brief This is called when the production is done
    /// this must materialize the production into the world
    /// @note example : spawn a unit
    virtual void produce(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const;
    /// @brief This is called when the production is enqueued
    /// @note example : consume resources
    virtual void enqueue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const;
    /// @brief This is called when the production is dequeued
    /// @note example : restore resources
    virtual void dequeue(flecs::entity producer_p, flecs::world const &ecs, custom_step_manager &manager_p) const;
    /// @brief return a unique name for this production
    virtual std::string name() const;
    /// @brief This is the duration (in steps) during which
    /// the production will be the current element of the queue
    /// before being produced.
    virtual int64_t duration() const;

private:
	std::string _name;
	int64_t duration_in_ticks = 0;
	std::unordered_map<std::string, octopus::Fixed> _resource_consumption;
	ResourceNodeEventBus* event_bus;
};

} // namespace godot
