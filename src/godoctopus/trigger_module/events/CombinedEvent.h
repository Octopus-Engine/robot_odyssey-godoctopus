#pragma once

#include "flecs.h"

template<typename FirstEvent, typename SecondEvent>
struct CombinedEvent
{
	static void apply(flecs::entity target, int32_t level)
	{
		FirstEvent::apply(target, level);
		SecondEvent::apply(target, level);
	}
};
