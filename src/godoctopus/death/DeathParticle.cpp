#include "DeathParticle.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/hitpoint/Destroyable.hh"
#include "godoctopus/projectile/CustomBasicProjectile.h"
#include "octopus_types.h"

void declare_death_particle_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particles) {
	ecs.observer<octopus::Destroyable const, octopus::Position const, ProjectileTrajectory const, DeathParticle const>()
		.event<octopus::Destroyed>()
		.each([particles](flecs::entity e, octopus::Destroyable const&, octopus::Position const &pos, ProjectileTrajectory const &proj, DeathParticle const &death) {
			particles->add_instance_detailed(
				WORLD_SCALE * Vector3(pos.pos.x.to_double(), proj.target_y.to_double()+0.25, pos.pos.y.to_double()),
				Color(death.r/255.,death.g/255.,death.b/255.,1.),
				death.count,
				death.scale * Vector3(1.,1.,1.),
				death.effect_id
			);
		});
}
