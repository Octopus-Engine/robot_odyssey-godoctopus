#include "CustomBasicProjectile.h"

#include "flecs.h"

#include "octopus/commands/basic/move/AttackCommand.hh"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"
#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"

#include "octopus_types.h"
#include <iostream>

void declare_basic_projectile_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particules) {

	ecs.component<CustomBasicProjectile>()
		.member("r", &CustomBasicProjectile::r)
		.member("g", &CustomBasicProjectile::g)
		.member("b", &CustomBasicProjectile::b)
		.member("origin_y", &CustomBasicProjectile::origin_y)
	;
	ecs.component<ProjectileTrajectory>()
		.member("target_y", &ProjectileTrajectory::target_y)
	;

	ecs.observer<octopus::Projectile const, octopus::ProjectileConstants const, CustomBasicProjectile const>()
			.event(flecs::OnSet)
			.each([] (flecs::entity e, octopus::Projectile const& proj, octopus::ProjectileConstants const& cst, CustomBasicProjectile const &info) {
				octopus::Fixed up = info.origin_y;
				octopus::Fixed end_up = 0.25;
				if (proj.target && proj.target.try_get<ProjectileTrajectory>()) {
					end_up = proj.target.try_get<ProjectileTrajectory>()->target_y;
				}
				float r=info.r/255., g=info.g/255., b=info.b/255.;
				e.set<SmartMMeshLibraryHandle>({0, r, g, b, 1., 0.2,
					up, // up
					end_up, // end_up
					octopus::get_time_stamp(e.world()),
					octopus::get_time_stamp(e.world()) + 20
				});
			});

	// pop damage
	ecs.system<octopus::ProjectileTrigger const, octopus::Projectile const, octopus::Position const, SmartMMeshLibraryHandle const>()
		.kind(ecs.entity(EndUpdatePhase))
		.with<CustomBasicProjectile>()
		.each([particules](flecs::entity e, octopus::ProjectileTrigger const&, octopus::Projectile const &,
				octopus::Position const &pos, SmartMMeshLibraryHandle const &handle) {
			particules->add_instance_detailed(
				WORLD_SCALE * Vector3(pos.pos.x.to_double(), handle.end_up.to_double(), pos.pos.y.to_double()),
				Color(handle.r.to_double(),handle.g.to_double(),handle.b.to_double(),1.),
				4,
				Vector3(0.5,0.5,0.5)
			);
		});

}
