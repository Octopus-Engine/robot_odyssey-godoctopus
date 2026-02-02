#include "HeavyfireBotProjectile.h"

#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/world/step/EntityCreationStep.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "godoctopus/components/special/Special.h"
#include "godoctopus/projectile/CustomBasicProjectile.h"

void declare_heavyfire_bot_projectile_systems(flecs::world &ecs, octopus::PositionContext const &ctx, custom_step_manager &manager) {
	using namespace octopus;

	// pop damage
	ecs.system<ProjectileTrigger const, Projectile const, HeavyfireBotProjectile const, Team const>()
		.kind(ecs.entity(EndUpdatePhase))
		.each([&manager, &ctx](flecs::entity e, ProjectileTrigger const& trigger, Projectile const &proj, HeavyfireBotProjectile const &heavy_proj, Team const &team) {
			if (trigger.target) {
				manager.get_last_layer().back().get<HitPointStep>().add_step(trigger.target, {-get_damage_after_armor(trigger.target, proj.damage) + heavy_proj.aoe_damage});
			}

			std::function<bool(int32_t, flecs::entity)> func_l = [&](int32_t idx_l, flecs::entity ent) -> bool {
				if(ent.try_get<Team>() &&
				   ent.try_get<Team>()->team != team.team &&
				   ent.try_get<HitPoint>() &&
				   ent.try_get<HitPoint>()->qty > Fixed::Zero()) {
					manager.get_last_layer().back().get<HitPointStep>().add_step(ent, HitPointStep{-heavy_proj.aoe_damage});
				}
				return true;
			};

			tree_circle_query(ctx.trees[0], proj.pos_target, heavy_proj.range, func_l);
		});

	// Spawn projectile
	ecs.system<Position const, AttackTrigger const, CustomBasicProjectile const, Attack const, Team const>()
		.kind(ecs.entity(EndUpdatePhase))
		.with<HeavyfireBot>()
		.each([&ecs](flecs::entity e, Position const &pos, AttackTrigger const& trigger, CustomBasicProjectile const &proj_data, Attack const &attack, Team const &team) {
			EntityCreationStep step_l;
			Projectile proj {trigger.target, octopus::Vector(), attack.cst.damage};
			if(trigger.target && trigger.target.try_get<Position>())
			{
				proj.pos_target = trigger.target.try_get<Position>()->pos;
			}
			HeavyfireBotProjectile proj_info {
				attack.cst.damage * (octopus::Fixed::One() / 10 + get_special_value(e) /100),
				10.
			};
			step_l.set_up_function = [pos, proj, proj_info, proj_data, team](flecs::entity new_ent, flecs::world const &world_p) {
				Position position;
				position.pos = pos.pos;
				new_ent.set<Position>(position)
					.set<Projectile>(proj)
					.set<ProjectileConstants>({20./TICK_RATE})
					.set<CustomBasicProjectile>(proj_data)
					.set<HeavyfireBotProjectile>(proj_info)
					.add<NoInstantDamage>()
					.set<Team>(team)
				;
			};

			ecs.try_get_mut<StepEntityManager>()->get_last_layer().push_back(step_l);
		});
}
