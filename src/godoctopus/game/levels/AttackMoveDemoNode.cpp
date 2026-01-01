#include "AttackMoveDemoNode.h"

#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/AttackCommandSystem.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/commands/queue/CommandQueue.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/position/Move.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"
#include "octopus/components/step/StepContainer.hh"

#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"
#include "godoctopus/pickable/Pickable.h"

#include "octopus_types.h"

/////////////////////////////////////////////////
/// This demo level aims at testing that attack can
/// retarget
/////////////////////////////////////////////////

namespace godot {

struct CustomBasicProjectile {};
struct CustomBasicProjectileInfo {
	int64_t r;
	int64_t g;
	int64_t b;
};
struct ProjectileTrajectory {
	octopus::Fixed origin_y;
	octopus::Fixed target_y;
};

void AttackMoveDemoNode::setup(Dictionary const &meta_data, GameNode &game) {
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

	ecs.component<CustomBasicProjectile>();
	ecs.component<CustomBasicProjectileInfo>()
		.member("r", &CustomBasicProjectileInfo::r)
		.member("g", &CustomBasicProjectileInfo::g)
		.member("b", &CustomBasicProjectileInfo::b)
	;
	ecs.component<ProjectileTrajectory>()
		.member("origin_y", &ProjectileTrajectory::origin_y)
		.member("target_y", &ProjectileTrajectory::target_y)
	;
	ecs.component<octopus::BasicProjectileAttack<CustomBasicProjectile>>()
		.member("speed", &octopus::BasicProjectileAttack<CustomBasicProjectile>::speed)
	;

	octopus::set_up_basic_projectile_systems<CustomBasicProjectile>(ecs);

	ecs.observer<octopus::Projectile const, octopus::ProjectileConstants const, CustomBasicProjectile const>()
			.event(flecs::OnAdd)
			.each([] (flecs::entity e, octopus::Projectile const& proj, octopus::ProjectileConstants const& cst, CustomBasicProjectile const &) {
				octopus::Fixed up = 1.5;
				octopus::Fixed end_up = 0.25;
				if (e && e.try_get<ProjectileTrajectory>()) {
					up = e.try_get<ProjectileTrajectory>()->origin_y;
				}
				if (proj.target && proj.target.try_get<ProjectileTrajectory>()) {
					end_up = proj.target.try_get<ProjectileTrajectory>()->target_y;
				}
				float r=22./256, g=90./256, b=76./256;
				auto info = e.try_get<CustomBasicProjectileInfo>();
				if (e && info) {
					r = info->r / 256.;
					g = info->g / 256.;
					b = info->b / 256.;
				}
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
		.each([this](flecs::entity e, octopus::ProjectileTrigger const&, octopus::Projectile const &,
				octopus::Position const &pos, SmartMMeshLibraryHandle const &handle) {
			if (_particules) {
				_particules->add_instance(WORLD_SCALE * Vector3(pos.pos.x.to_double(), handle.end_up.to_double(), pos.pos.y.to_double()), Color(22./256, 90./256, 76./256,1.));
			}
		});

	std::vector<flecs::entity> attackers;
	for(size_t i = 0 ; i < 500 ; ++ i) {
		auto e1 = ecs.entity()
			.add<custom_queue>()
			.set<octopus::Move>({5./TICK_RATE})
			.set<octopus::Position>({{50+0.01*i,100+0.01*i}, {0,0}, octopus::Fixed::One(), 0.5})
			.set<octopus::HitPoint>({50})
			.add<octopus::PositionInTree>()
			.set<octopus::Team>({1})
			.add<octopus::Destroyable>()
			.set<octopus::BasicProjectileAttack<CustomBasicProjectile>>({20./TICK_RATE})
			.set<octopus::AttackCommand>({flecs::entity()})
			.set<octopus::Attack>({{TICK_RATE/4, TICK_RATE, 15, 5}})
			.set<VatLibraryHandle>({2})
			.add<Pickable>()
			.set<ProjectileTrajectory>({1.5,1})
		;

		octopus::AttackCommand atk_l {flecs::entity(), {12,5}, true};
		e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
		e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionDone());
	}

	size_t n = 500;
	for(size_t i = 0 ; i < n/10 ; ++ i) {
		for(size_t j = 0 ; j < 10 ; ++ j) {
			auto e1 = ecs.entity()
				.add<custom_queue>()
				.set<octopus::Move>({3./TICK_RATE})
				.set<octopus::HitPoint>({150})
				.set<octopus::Team>({0})
				.add<octopus::Destroyable>()
				.add<octopus::PositionInTree>()
				.set<octopus::Position>({{12+0.5*i,5+0.5*j}, {0,0}, octopus::Fixed::One()})
				.set<octopus::AttackCommand>({flecs::entity()})
				.set<octopus::Attack>({{TICK_RATE/4, int32_t(1.5*TICK_RATE), 25, 1}})
				.set<VatLibraryHandle>({3})
				.add<Pickable>()
				.set<ProjectileTrajectory>({1.5,1})
			;

			octopus::AttackCommand atk_l {flecs::entity(), {50,100}, true};
			e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
			e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionDone());
		}
	}
}

void AttackMoveDemoNode::system_setup(Dictionary const &meta_data, GameNode &game) {

}

}