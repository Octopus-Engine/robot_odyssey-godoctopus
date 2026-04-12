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
#include "octopus/world/player/PlayerInfo.hh"

#include "godoctopus/components/proximity_custom_signal/ProximityCustomSignal.h"
#include "godoctopus/death/DeathParticle.h"
#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"
#include "godoctopus/game/ability/ArmorbotBuff.h"
#include "godoctopus/game/ability/BeaconSpawnAbility.h"
#include "godoctopus/game/ability/EarbotSteam.h"
#include "godoctopus/health_bar/HealthBarNode.h"
#include "godoctopus/pickable/Pickable.h"
#include "godoctopus/projectile/CustomBasicProjectile.h"
#include "godoctopus/projectile/HeavyfireBotProjectile.h"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "octopus_types.h"

/////////////////////////////////////////////////
/// This demo level aims at testing that attack can
/// retarget
/////////////////////////////////////////////////

namespace godot {

void declare_prefab(flecs::world &ecs) {
	// Add casting ability for earbots
	ecs.prefab("earbot")
		.auto_override<octopus::ResourceStock>()
		.auto_override<octopus::Caster>()
		.add<octopus::Caster>(ecs.component(EarbotSteam::NAME().c_str()));
	ecs.prefab("earlarge_bot")
		.auto_override<octopus::ResourceStock>()
		.auto_override<octopus::Caster>()
		.add<octopus::Caster>(ecs.component(EarbotSteam::NAME().c_str()));

	// Add casting ability for basic_resource
	ecs.prefab("basic_resource")
		.auto_override<octopus::ResourceStock>()
		.auto_override<octopus::Caster>()
		.set<ProximityCustomSignal>(ProximityCustomSignal {
			octopus::Fixed(20),
			"basic_resource_proximity",
			32,
			2
		})
		.add<octopus::Caster>(ecs.component(BeaconSpawnAbility::NAME().c_str()));

	// Add casting ability for armorbot
	ecs.prefab("armorbot")
		.auto_override<octopus::ResourceStock>()
		.auto_override<octopus::Caster>()
		.add<octopus::Caster>(ecs.component(ArmorbotAbility::NAME().c_str()));

	ecs.prefab("heavyfire_bot")
		.add<octopus::NoInstantDamage>()
	;
}

void AttackMoveDemoNode::setup(Dictionary const &meta_data, GameNode &game) {
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

	ecs.entity().set<octopus::PlayerInfo>({0, 0});
	ecs.entity().set<octopus::PlayerInfo>({1, 1});

	declare_prefab(ecs);
	declare_earbot_steam_ability(ecs, game.get_step_context());
	declare_armorbot_ability(ecs, game);
	declare_heavyfire_bot_projectile_systems(ecs, game.get_world().position_context, game.get_step_context().step_manager);

	for(int i = 0 ; i < count1 ; ++ i) {
		auto e1 = ecs.entity()
			.is_a(ecs.prefab(unit1.utf8().get_data()))
			.set<octopus::Team>({1})
			.set<octopus::PlayerAppartenance>({1})
			.set<octopus::Position>({{20+0.01*i,40+0.01*i}, {0,0}})
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
				.set<octopus::PlayerAppartenance>({0})
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

	declare_basic_projectile_systems(ecs, _particules, _particule_orchestrator);
	declare_attack_particule_systems(ecs, _vat_library, _particules);
	declare_windup_projectile_systems(ecs, _vat_library, _particules);
	declare_death_particle_systems(ecs, _particules);
	declare_triggers(ecs, game.get_world().position_context);
}

}
