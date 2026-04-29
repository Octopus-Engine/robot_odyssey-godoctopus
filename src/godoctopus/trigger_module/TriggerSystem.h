#pragma once

#include "octopus/components/basic/timestamp/TimeStamp.hh"
#include "octopus_types.h"

/// @brief Make an event occur when a trigger happens and a condition is met
/// @tparam Trigger The trigger triggering the event
/// @tparam Condition The condition to check before running the event
/// @tparam Event The event to run
/// @tparam Unit The unit or rune component that can trigger the event
template<typename Rune, typename Trigger, typename Condition, typename Event>
void declare_trigger_system(flecs::world &ecs)
{
	ecs.observer<Rune const, typename Trigger::component const>()
		.template event<Trigger>()
		.each([](flecs::entity e, Rune const &rune, typename Trigger::component const &) {
			if(Condition::check(e))
			{
				Event::apply(e, rune.level);
			}
		});
}

/// @brief Make an event occur when an attack happens and a condition is met
/// @tparam Trigger The trigger triggering the event
/// @tparam Condition The condition to check before running the event
/// @tparam Event The event to run, the event get the target of the attack
/// @tparam Unit The unit or rune component that can trigger the event
template<typename Rune, typename Trigger, typename Condition, typename Event>
void declare_attack_trigger_system(flecs::world &ecs)
{
	ecs.observer<Rune const, octopus::AttackTrigger const>()
		.template event<Trigger>()
		.each([](flecs::entity e, const Rune& rune, octopus::AttackTrigger const &trigger) {
			if(Condition::check(e))
			{
				Event::apply(e, trigger.target, rune.level);
			}
		});
}

/// @brief Make an event occur when an attack happens and a condition is met (for the target of the attack)
/// @tparam Trigger The trigger triggering the event
/// @tparam Condition The condition to check before running the event
/// @tparam Event The event to run, the event get NOT the attacker and the target of the attack
/// @tparam Unit The unit or rune component that can trigger the event
template<typename Rune, typename Trigger, typename Condition, typename Event>
void declare_attack_trigger_target_condition_system(flecs::world &ecs)
{
	ecs.observer<Rune const, octopus::AttackTrigger const>()
		.template event<Trigger>()
		.each([](flecs::entity e, const Rune& rune, octopus::AttackTrigger const &trigger) {
			if(Condition::check(trigger.target))
			{
				Event::apply(e, trigger.target, rune.level);
			}
		});
}

/// @brief Make an event occur when a trigger occurs and a condition is met, this event handle the position of the unit
/// and the position context to be able to query entites in a certain area
/// @tparam Trigger The trigger triggering the event
/// @tparam Condition The condition to check before running the event
/// @tparam Event The event to run, the event get the position of the unit and its team as well as the position context
/// @tparam Unit The unit or rune component that can trigger the event
template<typename Rune, typename Trigger, typename Condition, typename Event>
void declare_area_trigger_system(flecs::world &ecs, octopus::PositionContext const &ctx)
{
	ecs.observer<Rune const, typename Trigger::component const>()
		.template event<Trigger>()
		.each([&ctx](flecs::entity e, Rune const &rune, typename Trigger::component const &) {
			octopus::Position const *pos = e.try_get<octopus::Position>();
			octopus::Team const *team = e.try_get<octopus::Team>();
			if(pos && team && Condition::check(e))
			{
				Event::apply(e, pos->pos, team->team, ctx, rune.level);
			}
		});
}

template<typename Rune, typename Trigger, typename Condition, typename Event>
void declare_attack_area_trigger_system(flecs::world &ecs, octopus::PositionContext const &ctx)
{
	ecs.observer<Rune const, octopus::AttackTrigger const>()
		.template event<Trigger>()
		.each([&ctx](flecs::entity e, Rune const &rune, octopus::AttackTrigger const &trigger) {
			octopus::Position const *pos = trigger.target.try_get<octopus::Position>();
			octopus::Team const *team = e.try_get<octopus::Team>();
			if(pos && team && Condition::check(e))
			{
				Event::apply(e, pos->pos, team->team, ctx, rune.level);
			}
		});
}

template<typename Rune, typename Trigger, typename Condition, typename Event>
void declare_attack_area_trigger_target_condition_system(flecs::world &ecs, octopus::PositionContext const &ctx)
{
	ecs.observer<Rune const, octopus::AttackTrigger const>()
		.template event<Trigger>()
		.each([&ctx](flecs::entity e, Rune const &rune, octopus::AttackTrigger const &trigger) {
			octopus::Position const *pos = trigger.target.try_get<octopus::Position>();
			octopus::Team const *team = e.try_get<octopus::Team>();
			if(pos && team && Condition::check(trigger.target))
			{
				Event::apply(e, pos->pos, team->team, ctx, rune.level);
			}
		});
}

/// @brief Tracking component for periodic trigger timing
template<typename Rune>
struct PulseRuneTriggerTime
{
	int64_t last_trigger_time = -TICK_RATE;  // Initialize to trigger on first check
};

/// @brief Periodic trigger system that applies an AoE effect every N ticks
/// Requires the entity to have the Rune component, and will check conditions and apply events on intervals
template<typename Rune, typename Condition, typename Event>
void declare_periodic_area_trigger_system(flecs::world &ecs, octopus::PositionContext const &ctx, int64_t tick_interval = TICK_RATE)
{
	// Register timing tracking component
	ecs.component<PulseRuneTriggerTime<Rune>>();

	ecs.system<Rune, octopus::Position, octopus::Team, PulseRuneTriggerTime<Rune> *>()
		.kind(ecs.entity(UpdatePhase))
		.each([&ctx, tick_interval](flecs::entity e, Rune const &rune, octopus::Position const &pos, octopus::Team const &team, PulseRuneTriggerTime<Rune> *trigger_time) {
			if(!Condition::check(e))
			{
				return;
			}

			int64_t current_time = octopus::get_time_stamp(e.world());

			// Check if enough ticks have passed since last trigger
			if(!trigger_time || current_time - trigger_time->last_trigger_time >= tick_interval) {
				if(trigger_time) {
					trigger_time->last_trigger_time = current_time;
				} else {
					e.set<PulseRuneTriggerTime<Rune>>({current_time});
				}
				Event::apply(e, pos.pos, team.team, ctx, rune.level);
			}
		});
}
