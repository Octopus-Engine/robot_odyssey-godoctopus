#include "AttackMoveDemoNode.h"

#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/commands/queue/CommandQueue.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/position/Move.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"
#include "octopus/components/step/StepContainer.hh"

#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"
#include "godoctopus/pickable/Pickable.h"
#include "godoctopus/projectile/CustomBasicProjectile.h"
#include "godoctopus/health_bar/HealthBarNode.h"

#include "octopus_types.h"

/////////////////////////////////////////////////
/// This demo level aims at testing that attack can
/// retarget
/////////////////////////////////////////////////

namespace godot {

void AttackMoveDemoNode::setup(Dictionary const &meta_data, GameNode &game) {
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

	ecs.prefab("rambot")
		.auto_override<custom_queue>()
		.set_auto_override<octopus::Move>({3./TICK_RATE})
		.set_auto_override<octopus::HitPoint>({150})
		.set_auto_override<octopus::HitPointMax>({150})
		.auto_override<octopus::Destroyable>()
		.set<octopus::Collision>({3*octopus::Fixed::One()/4})
		.auto_override<octopus::PositionInTree>()
		.set_auto_override<octopus::AttackCommand>({flecs::entity()})
		.set_auto_override<octopus::Attack>({{TICK_RATE/4, int32_t(1.5*TICK_RATE), 25, 1}})
		.set_auto_override<VatLibraryHandle>({3})
		.auto_override<Pickable>()
		.set_auto_override<ProjectileTrajectory>({1})
		.set_auto_override<HealthBar>({2., 48.})
	;

	ecs.prefab("tallbot")
		.auto_override<custom_queue>()
		.set_auto_override<octopus::Move>({5./TICK_RATE})
		.set_auto_override<octopus::HitPoint>({75})
		.set_auto_override<octopus::HitPointMax>({75})
		.auto_override<octopus::Destroyable>()
		.auto_override<octopus::PositionInTree>()
		.set<octopus::Collision>({octopus::Fixed::One()/2})
		.set_auto_override<octopus::BasicProjectileAttack<CustomBasicProjectile>>({20./TICK_RATE, {22,90,76,1.5}})
		.set_auto_override<octopus::AttackCommand>({flecs::entity()})
		.set_auto_override<octopus::Attack>({{TICK_RATE/4, TICK_RATE, 15, 7}})
		.set_auto_override<VatLibraryHandle>({2})
		.auto_override<Pickable>()
		.set_auto_override<ProjectileTrajectory>({1})
		.set_auto_override<HealthBar>({2., 32.})
	;

	ecs.prefab("earbot")
		.auto_override<custom_queue>()
		.set_auto_override<octopus::Move>({5./TICK_RATE})
		.set_auto_override<octopus::HitPoint>({75})
		.set_auto_override<octopus::HitPointMax>({75})
		.auto_override<octopus::Destroyable>()
		.set<octopus::Collision>({octopus::Fixed::One()/2})
		.auto_override<octopus::PositionInTree>()
		.set_auto_override<octopus::BasicProjectileAttack<CustomBasicProjectile>>({20./TICK_RATE, {251,185,84,1, 0.2}})
		.set_auto_override<octopus::AttackCommand>({flecs::entity()})
		.set_auto_override<octopus::Attack>({{TICK_RATE/4, TICK_RATE/2, 10, 7}})
		.set_auto_override<VatLibraryHandle>({0})
		.auto_override<Pickable>()
		.set_auto_override<ProjectileTrajectory>({0.5})
		.set_auto_override<HealthBar>({2., 32.})
	;

	for(int i = 0 ; i < count1 ; ++ i) {
		auto e1 = ecs.entity()
			.is_a(ecs.prefab(unit1.utf8().get_data()))
			.set<octopus::Team>({1})
			.set<octopus::Position>({{50+0.01*i,100+0.01*i}, {0,0}})
		;

		octopus::AttackCommand atk_l {flecs::entity(), {12,5}, true};
		e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
		e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionDone());
	}

	int n = count2;
	for(int i = 0 ; i < n/10 ; ++ i) {
		for(int j = 0 ; j < 10 ; ++ j) {
			auto e1 = ecs.entity()
				.is_a(ecs.prefab(unit2.utf8().get_data()))
				.set<octopus::Team>({0})
				.set<octopus::Position>({{12+0.5*i,5+0.5*j}, {0,0}})
			;

			octopus::AttackCommand atk_l {flecs::entity(), {50,100}, true};
			e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
			e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionDone());
		}
	}
}

void AttackMoveDemoNode::system_setup(Dictionary const &meta_data, GameNode &game) {
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

	declare_basic_projectile_systems(ecs, _particules);

	if (_particules) {
		ecs.observer<octopus::Destroyable const, octopus::Position const, ProjectileTrajectory const>()
			.event<octopus::Destroyed>()
			.each([this](flecs::entity e, octopus::Destroyable const&, octopus::Position const &pos, ProjectileTrajectory const &proj) {
				_particules->add_instance_detailed(
					WORLD_SCALE * Vector3(pos.pos.x.to_double(), proj.target_y.to_double()+0.25, pos.pos.y.to_double()),
					Color(1.,1.,1.,1.),
					8,
					Vector3(1.5,1.5,1.5)
				);
			});
		}
}

}
