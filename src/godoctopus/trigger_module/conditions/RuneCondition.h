#pragma once

#include "flecs.h"
#include "godoctopus/components/rune_load/RuneLoad.h"

/// @brief Checks if an entity possesses a sufficient quantity of a specific rune type and consumes them if the condition is met.
/// @tparam requirement The quantity of runes required.
/// @tparam type The type of rune.
/// @tparam consume If true, the runes are consumed after checking the condition.
template<int32_t requirement, typename type, bool  consume=true>
struct RuneCondition
{
	static bool check(flecs::entity e)
	{
		RuneLoad<type> * rune_load = e.try_get_mut<RuneLoad<type>>();
		return rune_load && rune_load->qty >= requirement;
	}

	static void post_condition(flecs::entity e) {
		if (!consume) {
			return;
		}
		RuneLoad<type> * rune_load = e.try_get_mut<RuneLoad<type>>();
		if(rune_load && requirement > 0) {
			rune_load->qty -= requirement;
			e.world().event<trigger_module::RuneConsumed<type>>()
				.template id<RuneLoad<type>>()
				.entity(e)
				.emit();
		}
	}
};
