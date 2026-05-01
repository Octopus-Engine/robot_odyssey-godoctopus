#include "AoePulseRune.h"

#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/conditions/AlwaysCondition.h"
#include "godoctopus/trigger_module/events/HitpointEvent.h"

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
