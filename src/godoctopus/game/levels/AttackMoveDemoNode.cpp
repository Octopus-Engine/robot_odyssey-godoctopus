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
#include "godoctopus/projectile/CustomBasicProjectile.h"

#include "octopus_types.h"

/////////////////////////////////////////////////
/// This demo level aims at testing that attack can
/// retarget
/////////////////////////////////////////////////

namespace godot {

void AttackMoveDemoNode::setup(Dictionary const &meta_data, GameNode &game) {
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

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
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

	octopus::set_up_basic_projectile_systems<CustomBasicProjectile>(ecs);

	if (_particules) {
		declare_basic_projectile_systems(ecs, _particules);
	}
}

}
