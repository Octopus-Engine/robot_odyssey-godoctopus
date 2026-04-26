#include "CustomBasicProjectile.h"

#include "flecs.h"

#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/AttackCommandSystem.hh"
#include "octopus/serialization/containers/VectorSupport.hh"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"
#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"

#include "octopus_types.h"
#include <iostream>

void declare_windup_projectile_systems(flecs::world &ecs, godot::VatLibrary *library, godot::ParticuleSmartMMesh *particules) {
	ecs.component<WindupEffect>()
		.member("effect_id_load", &WindupEffect::effect_id_load)
		.member("effect_id_incoming", &WindupEffect::effect_id_incoming)
		.member("x", &WindupEffect::x)
		.member("y", &WindupEffect::y)
		.member("z", &WindupEffect::z)
		.member("r", &WindupEffect::r)
		.member("g", &WindupEffect::g)
		.member("b", &WindupEffect::b)
		.member("count", &WindupEffect::count)
		.member("scale", &WindupEffect::scale)
	;

	ecs.system<octopus::Position const, octopus::Attack const, octopus::AttackCommand const, VatLibraryHandle const, WindupEffect const>()
		.kind(ecs.entity(DisplaySyncPhase))
		.each([library, particules](flecs::entity e, octopus::Position const &pos,
		octopus::Attack const &attack, octopus::AttackCommand const& attackCommand, VatLibraryHandle const &handle, WindupEffect const &effect) {
			if(handle.instance_id < 0 ) { return; }
			if(attack.windup == 1) {
				Vector3 position = get_transformed_position(Vector3(effect.x,effect.y,effect.z), library, handle);
				particules->add_instance_load(
					WORLD_SCALE * position,
					Color(effect.r/255., effect.g/255.,effect.b/255.,1.),
					effect.count,
					Vector3(1.,1.,1.)*effect.scale,
					effect.effect_id_load,
					effect.effect_id_incoming
				);
			}
		});
}

void declare_attack_particule_systems(flecs::world &ecs, godot::VatLibrary *library, godot::ParticuleSmartMMesh *particules) {

	ecs.component<AttackParticle>()
		.member("effect_id",&AttackParticle::effect_id)
		.member("x",&AttackParticle::x)
		.member("y",&AttackParticle::y)
		.member("z",&AttackParticle::z)
		.member("r",&AttackParticle::r)
		.member("g",&AttackParticle::g)
		.member("b",&AttackParticle::b)
		.member("count",&AttackParticle::count)
		.member("scale",&AttackParticle::scale)
	;

	ecs.system<octopus::Position const, octopus::AttackTrigger const, AttackParticle const, VatLibraryHandle const>()
		.kind(ecs.entity(EndUpdatePhase))
		.each([library, particules](flecs::entity e, octopus::Position const &pos, octopus::AttackTrigger const& trigger, AttackParticle const &info, VatLibraryHandle const &handle) {
			if(!trigger.target || !trigger.target.try_get<octopus::Position>()) { return; }
			octopus::Fixed end_up = 0.25;
			if (trigger.target.try_get<ProjectileTrajectory>()) {
				end_up = trigger.target.try_get<ProjectileTrajectory>()->target_y;
			}

			octopus::Vector const &target_pos_2d = trigger.target.try_get<octopus::Position>()->pos * WORLD_SCALE;
			Vector3 target_pos(target_pos_2d.x.to_double(), end_up.to_double(), target_pos_2d.y.to_double());
			Vector3 source_pos = get_transformed_position(Vector3(info.x,info.y,info.z), library, handle);
			Vector3 direction = target_pos - source_pos;

			float r=info.r/255., g=info.g/255., b=info.b/255.;
			particules->add_instance_coned(
				WORLD_SCALE * source_pos,
				Color(r, g, b,1.),
				info.count,
				info.scale*Vector3(1.,1.,1.),
				direction,
				20.,
				info.effect_id
			);
		});
}

void declare_basic_projectile_systems(flecs::world &ecs, godot::ParticuleSmartMMesh *particules, godot::ParticleOrchestrator *particule_orchestrator, octopus::WorldContext<custom_step_manager> &world_context) {
    // Register reflection for std::vector<float>
    ecs.component<std::vector<float>>()
        .opaque(std_vector_support<float>);

	ecs.component<CustomBasicProjectile::Impact>()
		.member("effect_idx", &CustomBasicProjectile::Impact::effect_idx)
		.member("color", &CustomBasicProjectile::Impact::color);

    // Register reflection for std::vector<CustomBasicProjectile::Impact>
    ecs.component<std::vector<CustomBasicProjectile::Impact>>()
        .opaque(std_vector_support<CustomBasicProjectile::Impact>);

	ecs.component<CustomBasicProjectile>()
		.member("r", &CustomBasicProjectile::r)
		.member("g", &CustomBasicProjectile::g)
		.member("b", &CustomBasicProjectile::b)
		.member("origin_y", &CustomBasicProjectile::origin_y)
		.member("scale", &CustomBasicProjectile::scale)
		.member("impacts", &CustomBasicProjectile::impacts)
		.member("impact_effect_id", &CustomBasicProjectile::impact_effect_id)
		.member("impact_count", &CustomBasicProjectile::impact_count)
		.member("impact_scale", &CustomBasicProjectile::impact_scale)
	;
	ecs.component<ProjectileTrajectory>()
		.member("target_y", &ProjectileTrajectory::target_y)
	;

	octopus::set_up_basic_projectile_basis(ecs);
	octopus::set_up_basic_projectile_systems<CustomBasicProjectile>(ecs, world_context);

	ecs.observer<octopus::Projectile const, octopus::ProjectileConstants const, CustomBasicProjectile const, octopus::Position const>()
			.event(flecs::OnSet)
			.each([particules] (flecs::entity e, octopus::Projectile const& proj, octopus::ProjectileConstants const& cst, CustomBasicProjectile const &info, octopus::Position const &pos) {
				octopus::Fixed up = info.origin_y;
				octopus::Fixed end_up = 0.25;
				if (proj.target && proj.target.try_get<ProjectileTrajectory>()) {
					end_up = proj.target.try_get<ProjectileTrajectory>()->target_y;
				}
				float r=info.r/255., g=info.g/255., b=info.b/255.;
				e.set<ProjectileSmartMMesh>({r, g, b, 1., info.scale,
					up, // up
					end_up, // end_up
					octopus::get_time_stamp(e.world()),
					octopus::get_time_stamp(e.world()) + 20
				});
				e.set<SmartMMeshLibraryHandle>({0});
			});

	if (particules && particule_orchestrator) {
		// pop damage
		ecs.system<octopus::ProjectileTrigger const, octopus::Projectile const, octopus::Position const, ProjectileSmartMMesh const, CustomBasicProjectile const>()
			.kind(ecs.entity(EndUpdatePhase))
			.each([particules, particule_orchestrator](flecs::entity e, octopus::ProjectileTrigger const&, octopus::Projectile const &,
					octopus::Position const &pos, ProjectileSmartMMesh const& proj, CustomBasicProjectile const &info) {
				// generic
				Vector3 effect_position = WORLD_SCALE * Vector3(pos.pos.x.to_double(), proj.end_up.to_double(), pos.pos.y.to_double());
				// specific
				if (info.impacts.size() > 0) {
					for (auto const &impact : info.impacts) {
						godot::ParticuleTypeData particule_data {
							Color(impact.color[0],impact.color[1],impact.color[2],impact.color[3]).srgb_to_linear(),
							Vector3(1.,1.,1.),			// scale
							impact.effect_idx,			// resource
							effect_position,			// position
							Vector3(0.,0.,0.),			// direction
							0.,							// time_offset
							(std::rand() % 1024) / 1024. * 90. // rot_y
						};
						particule_orchestrator->add_particle(std::move(particule_data));
							// effect_position, Color(impact.color[0],impact.color[1],impact.color[2],impact.color[3]), impact.effect_idx);
					}
				} else {
					particules->add_instance_detailed(
						effect_position,
						Color(proj.r.to_double(), proj.g.to_double(),proj.b.to_double(),1.),
						info.impact_count,
						info.impact_scale * Vector3(1.,1.,1.),
						info.impact_effect_id
					);
				}
			});
	}
}
