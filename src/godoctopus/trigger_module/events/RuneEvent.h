#pragma once

#include "flecs.h"

#include "octopus/world/position/PositionContext.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"

#include "godoctopus/components/rune_load/RuneLoad.h"

template<typename type, int32_t delta>
struct RuneEvent
{
	static void apply(flecs::entity target, int32_t level=0)
	{
		RuneLoad<type> * rune_load = target.try_get_mut<RuneLoad<type>>();
		if(rune_load)
		{
			rune_load->qty += delta;
			rune_load->last_update_time = octopus::get_time_stamp(target.world());
			if(delta > 0)
			{
				target.world().event<trigger_module::RuneLoaded<type>>()
					.template id<RuneLoad<type>>()
					.entity(target)
					.emit();
			}
			else if(delta < 0)
			{
				target.world().event<trigger_module::RuneConsumed<type>>()
					.template id<RuneLoad<type>>()
					.entity(target)
					.emit();
			}
		}
	}
};


template<typename type, int32_t delta, int32_t range, bool ally>
struct RuneAreaEvent
{
	static void apply(flecs::entity target, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx, int32_t level=0)
	{
		using namespace octopus;

		auto func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
			if((ally && ent.try_get<Team>() && ent.try_get<Team>()->team == team && ent.try_get<HitPoint>())
			|| (!ally && ent.try_get<Team>() && ent.try_get<Team>()->team != team && ent.try_get<HitPoint>()))
			{
				RuneLoad<type> * rune_load = ent.try_get_mut<RuneLoad<type>>();
				rune_load->qty += delta;
				rune_load->last_update_time = octopus::get_time_stamp(ent.world());
				if(delta > 0)
				{
					ent.world().event<trigger_module::RuneLoaded<type>>()
						.template id<RuneLoad<type>>()
						.entity(ent)
						.emit();
				}
				else if(delta < 0)
				{
					ent.world().event<trigger_module::RuneConsumed<type>>()
						.template id<RuneLoad<type>>()
						.entity(ent)
						.emit();
				}
			}
			return true;
		};

		tree_circle_query<flecs::entity>(ctx.trees[0], center, range, func_l);
	}
};
