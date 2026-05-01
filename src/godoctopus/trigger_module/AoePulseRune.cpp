#include "AoePulseRune.h"

#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/conditions/AlwaysCondition.h"
#include "godoctopus/trigger_module/events/HitpointEvent.h"

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

void declare_aoe_pulse_triggers(flecs::world &ecs, octopus::PositionContext const &ctx)
{
	// declare periodic area trigger systems for pulse runes
	// Tier 1: base stats (5% base, 3% per level)
	declare_periodic_area_trigger_system<AoePulseHealBasedOnHitpoint, AlwaysCondition, HealAreaEventPeriodicHitpointBased<5, 3, 3>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseHealBasedOnDamage, AlwaysCondition, HealAreaEventPeriodicDamageBased<5, 3, 3>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnHitpoint, AlwaysCondition, DamageAreaEventPeriodicHitpointBased<5, 3, 3>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnDamage, AlwaysCondition, DamageAreaEventPeriodicDamageBased<5, 3, 3>>(ecs, ctx);

	// Tier 2: enhanced stats (10% base, 5% per level)
	declare_periodic_area_trigger_system<AoePulseHealBasedOnHitpointTier2, AlwaysCondition, HealAreaEventPeriodicHitpointBased<10, 3, 5>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseHealBasedOnDamageTier2, AlwaysCondition, HealAreaEventPeriodicDamageBased<10, 3, 5>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnHitpointTier2, AlwaysCondition, DamageAreaEventPeriodicHitpointBased<10, 3, 5>>(ecs, ctx);
	declare_periodic_area_trigger_system<AoePulseDamageBasedOnDamageTier2, AlwaysCondition, DamageAreaEventPeriodicDamageBased<10, 3, 5>>(ecs, ctx);
}
