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

#include "octopus_types.h"

/////////////////////////////////////////////////
/// This demo level aims at testing that attack can
/// retarget
/////////////////////////////////////////////////

namespace godot {

struct CustomProj {};

void AttackMoveDemoNode::setup(Dictionary const &meta_data, GameNode &game) {
	flecs::world &ecs = game.get_world().ecs;

	octopus::set_up_basic_projectile_systems<CustomProj>(ecs);

	ecs.observer<octopus::Projectile const, octopus::ProjectileConstants const, CustomProj const>()
			.event(flecs::OnAdd)
			.each([] (flecs::entity e, octopus::Projectile const& proj, octopus::ProjectileConstants const& cst, CustomProj const &) {
				e.set<SmartMMeshLibraryHandle>({0,22./256, 90./256, 76./256,1.,0.2,
					1.5, // up
					0.25, // end_up
					octopus::get_time_stamp(e.world()),
					octopus::get_time_stamp(e.world()) + 20
				});
			});

	std::vector<flecs::entity> attackers;
	for(size_t i = 0 ; i < 50 ; ++ i) {
		auto e1 = ecs.entity()
			.add<custom_queue>()
			.set<octopus::Move>({5./TICK_RATE})
			.set<octopus::Position>({{10+0.01*i,40+0.01*i}, {0,0}, octopus::Fixed::One()})
			.add<octopus::PositionInTree>()
			.set<octopus::Team>({1})
			.add<octopus::Destroyable>()
			.set<octopus::BasicProjectileAttack<CustomProj>>({20./TICK_RATE})
			.set<octopus::AttackCommand>({flecs::entity()})
			.set<octopus::Attack>({{TICK_RATE/4, TICK_RATE, 5, 5}})
			.set<VatLibraryHandle>({2})
		;
		attackers.push_back(e1);
	}

	auto e2 = ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{13,5}, {0,0}, octopus::Fixed::One()})
		.set<VatLibraryHandle>({1})
	;

	for(size_t i = 0 ; i < 100 ; ++ i) {
		ecs.entity()
			.add<custom_queue>()
			.add<octopus::Move>()
			.set<octopus::HitPoint>({10})
			.set<octopus::Team>({0})
			.add<octopus::Destroyable>()
			.add<octopus::PositionInTree>()
			.set<octopus::Position>({{12+0.1*i,5+0.1*i}, {0,0}, octopus::Fixed::One()})
			.set<VatLibraryHandle>({1})
		;
	}

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({1})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{7,5}, {0,0}, octopus::Fixed::One()})
		.set<VatLibraryHandle>({0})
	;

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{10,3}, {0,0}, octopus::Fixed::One()})
		.set<VatLibraryHandle>({1})
	;

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{7,0}, {0,0}, octopus::Fixed::One()})
		.set<VatLibraryHandle>({1})
	;

	for(auto & e1 : attackers)
	{
		octopus::AttackCommand atk_l {e2, {10,0}};
		e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
		e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionDone());
	}
}

void AttackMoveDemoNode::system_setup(Dictionary const &meta_data, GameNode &game) {

}

}