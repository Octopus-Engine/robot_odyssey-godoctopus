#pragma once

template<typename Event>
struct TargetEvent
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t level, int32_t percent, int32_t upgrade_percent)
	{
		Event::apply(target, level, percent, upgrade_percent);
	}
};

template<typename Event>
struct AttackerEvent
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t level, int32_t percent, int32_t upgrade_percent)
	{
		Event::apply(attacker, level, percent, upgrade_percent);
	}
};
