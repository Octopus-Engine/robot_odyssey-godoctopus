#include "apply_hit_point_delta_area.h"

#include "godoctopus/on_screen_ui/DamageTextNode.h"

void apply_hit_point_delta_area(flecs::entity source, octopus::Vector const &center, uint16_t team, octopus::PositionContext const &ctx,
	octopus::Fixed range, octopus::Fixed delta)
{
	using namespace octopus;

	auto apply_delta = [&](int32_t idx_l, flecs::entity ent) -> bool {
		const bool check_team = ent.try_get<Team>() && (delta > 0
			? ent.try_get<Team>()->team == team
			: ent.try_get<Team>()->team != team);
		if(check_team && ent.try_get<HitPoint>()) {
			HitPoint * hp = ent.try_get_mut<HitPoint>();
			hp->qty += delta;
			trigger_module::emit_damage_text_trigger(ent, std::abs(delta), delta > 0);
		}
		return true;
	};

	tree_circle_query<flecs::entity>(ctx.trees[0], center, range, apply_delta);
}
