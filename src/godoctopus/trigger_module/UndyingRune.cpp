#include "UndyingRune.h"

#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/systems/phases/Phases.hh"

#include "godoctopus/trigger_module/events/TemporaryBuffEvent.h"
#include "godoctopus/trigger_module/UndyingRune.h"
#include "godoctopus/trigger_module/BuffDeclarer.h"
#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "godoctopus/trigger_module/buffs/TemporaryUndyingBuff.h"
#include "godoctopus/trigger_module/conditions/RuneCondition.h"
#include "godoctopus/trigger_module/conditions/AndCondition.h"
#include "godoctopus/trigger_module/conditions/HasComponentCondition.h"
#include "godoctopus/trigger_module/UndyingBuffCooldown.h"
#include "godoctopus/components/special/Special.h"

// Forward declare the typedef for Undying Buff
typedef TemporaryUndyingBuff<15*TICK_RATE> TemporaryUndyingBuff_15s;

void declare_undying_rune_triggers(flecs::world &ecs, custom_step_manager& manager)
{
	declare_trigger_buff<ApplyUndyingBuffOnRuneLoad>(ecs);

	// Condition for 5 rune loads
	using RuneLoadCondition = RuneCondition<5, DefaultRune>;
	// Add a condition for 5 rune loads AND the absence of the cooldown
	using BuffCondition = AndCondition<RuneLoadCondition, HasComponentCondition<UndyingBuffCooldown, true>>;
	// Undying buff trigger: when rune load reaches 5, apply the undying buff
	declare_trigger_system<ApplyUndyingBuffOnRuneLoad, trigger_module::RuneLoaded<DefaultRune>, BuffCondition, ApplyTemporaryBuffComponentEvent<TemporaryUndyingBuff_15s>>(ecs);

	// Register the Undying Buff component and systems
	ecs.component<TemporaryUndyingBuff_15s>()
		.member("decoy", &TemporaryUndyingBuff_15s::decoy);

	octopus::declare_buff_system<TemporaryUndyingBuff_15s>(ecs, manager);

	// ValidatePhase system: Clamp HitPoint to minimum 1 when Undying buff is active
	ecs.system<octopus::HitPoint>()
		.with<TemporaryUndyingBuff_15s>()
		.multi_threaded()
		.kind(ecs.entity(ValidatePhase))
		.each([](octopus::HitPoint &hp_p) {
			if(hp_p.qty <= octopus::Fixed::Zero()) {
				hp_p.qty = octopus::Fixed::One();
			}
		});

	// Register UndyingBuffCooldown component
	ecs.component<UndyingBuffCooldown>()
		.member("cooldown_end_tick", &UndyingBuffCooldown::cooldown_end_tick)
	;

	// Observer for when TemporaryUndyingBuff_15s is added (applied)
	ecs.observer<>()
		.event(flecs::OnAdd)
		.with<TemporaryUndyingBuff_15s>()
		.each([&ecs](flecs::entity e) {
			// When buff is applied, apply cooldown
			// Calculate duration: (15 * TICK_RATE) / (1 + special_value * 0.1)
			Special const *special = e.try_get<Special>();
			if(special)
			{
				int64_t base_cooldown_ticks = 15 * TICK_RATE + TemporaryUndyingBuff_15s::DURATION_TICKS;
				// Convert special value to integer for division
				int64_t special_int = static_cast<int64_t>((special->value * 0.1).to_int());
				// Avoid division by zero
				int64_t divisor = 1 + special_int;
				int64_t cooldown_duration = base_cooldown_ticks / divisor;

				int64_t current_tick = octopus::get_time_stamp(e.world());
				int64_t cooldown_end_tick = current_tick + cooldown_duration;

				// Apply cooldown and prevention components
				e.set<UndyingBuffCooldown>({cooldown_end_tick});
			}
		});

	// System to monitor cooldown expiration and remove the cooldown component
	ecs.system<UndyingBuffCooldown>()
		.multi_threaded()
		.kind(ecs.entity(ValidatePhase))
		.each([](flecs::entity e, UndyingBuffCooldown const &cooldown) {
			int64_t current_tick = octopus::get_time_stamp(e.world());
			if(current_tick >= cooldown.cooldown_end_tick) {
				// Cooldown has expired, remove the component (triggers OnRemove observer)
				e.remove<UndyingBuffCooldown>();
			}
		});

	// Observer for when UndyingBuffCooldown is removed
	// This triggers when the cooldown expires
	ecs.observer<>()
		.event(flecs::OnRemove)
		.with<UndyingBuffCooldown>()
		.each([&ecs](flecs::entity e) {
			// Cooldown expired, check if we can reapply the buff
			// Check if entity has the rune loaded (ApplyUndyingBuffOnRuneLoad present)
			ApplyUndyingBuffOnRuneLoad const *rune = e.try_get<ApplyUndyingBuffOnRuneLoad>();
			if(rune && RuneLoadCondition::check(e)) {
				// Conditions are met, reapply the buff
				ApplyTemporaryBuffComponentEvent<TemporaryUndyingBuff_15s>::apply(e, rune->level);
			}
		});
}
