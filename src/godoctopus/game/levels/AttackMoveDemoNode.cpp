#include "AttackMoveDemoNode.h"

#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/commands/queue/CommandQueue.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/position/Move.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/step/StepContainer.hh"

#include "godoctopus/display/vat/VatLibraryHandle.h"

#include "octopus_types.h"

/////////////////////////////////////////////////
/// This demo level aims at testing that attack can
/// retarget
/////////////////////////////////////////////////

namespace godot {

void AttackMoveDemoNode::setup(Dictionary const &meta_data, GameNode &game) {
	flecs::world &ecs = game.get_world().ecs;

	auto e1 = ecs.entity()
		.add<custom_queue>()
		.set<octopus::Move>({5./TICK_RATE})
		.set<octopus::Position>({{10,20}, {0,0}, octopus::Fixed::One(), octopus::Fixed::Zero(), false})
		.add<octopus::PositionInTree>()
		.set<octopus::Team>({1})
		.add<octopus::Destroyable>()
		.set<octopus::AttackCommand>({flecs::entity()})
		.set<octopus::Attack>({{TICK_RATE/2, TICK_RATE, 5, 2}})
		.set<VatLibraryHandle>({2})
	;

	auto e2 = ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{13,5}, {0,0}, octopus::Fixed::One(), octopus::Fixed::Zero(), false})
		.set<VatLibraryHandle>({1})
	;

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{12,5}, {0,0}, octopus::Fixed::One(), octopus::Fixed::Zero(), false})
		.set<VatLibraryHandle>({1})
	;

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({1})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{7,5}, {0,0}, octopus::Fixed::One(), octopus::Fixed::Zero(), false})
		.set<VatLibraryHandle>({0})
	;

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{10,3}, {0,0}, octopus::Fixed::One(), octopus::Fixed::Zero(), false})
		.set<VatLibraryHandle>({1})
	;

	ecs.entity()
		.add<custom_queue>()
		.add<octopus::Move>()
		.set<octopus::HitPoint>({10})
		.set<octopus::Team>({0})
		.add<octopus::Destroyable>()
		.add<octopus::PositionInTree>()
		.set<octopus::Position>({{7,0}, {0,0}, octopus::Fixed::One(), octopus::Fixed::Zero(), false})
		.set<VatLibraryHandle>({1})
	;

	octopus::AttackCommand atk_l {e2, {10,0}};
	e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
	e1.try_get_mut<custom_queue>()->_queuedActions.push_back(octopus::CommandQueueActionDone());
}

void AttackMoveDemoNode::system_setup(Dictionary const &meta_data, GameNode &game) {

}

}