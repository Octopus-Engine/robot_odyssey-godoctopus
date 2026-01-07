#pragma once

template<typename Event>
struct TargetEvent
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t level)
	{
		Event::apply(target, level);
	}
};

template<typename Event>
struct AttackerEvent
{
	static void apply(flecs::entity attacker, flecs::entity target, int32_t level)
	{
		Event::apply(attacker, level);
	}
};
