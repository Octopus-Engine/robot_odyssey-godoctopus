#include "AttackMoveDemoNode.h"

#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/commands/queue/CommandQueue.hh"
#include "octopus/components/advanced/production/queue/ProductionQueue.hh"
#include "octopus/components/advanced/production/PlayerProduction.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/position/Move.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/timestamp/TimeStamp.hh"
#include "octopus/components/step/StepContainer.hh"
#include "octopus/world/player/PlayerInfo.hh"

#include "godoctopus/components/player/PlayerLoadout.h"
#include "godoctopus/components/proximity_custom_signal/ProximityCustomSignal.h"
#include "godoctopus/death/DeathParticle.h"
#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"
#include "godoctopus/game/ability/unit/ArmorbotBuff.h"
#include "godoctopus/game/ability/building/ProximityBeaconSpawnAbility.h"
#include "godoctopus/game/ability/building/BasicResourceProducerBeaconSpawnAbility.h"
#include "godoctopus/game/ability/building/AdvancedResourceProducerBeaconSpawnAbility.h"
#include "godoctopus/game/ability/building/UnitProducerBeaconSpawnAbility.h"
#include "godoctopus/game/ability/unit/EarbotSteam.h"
#include "godoctopus/game/player_buffs/PlayerDamageBuff.h"
#include "godoctopus/game/player_buffs/PlayerHealthBuff.h"
#include "godoctopus/health_bar/HealthBarNode.h"
#include "godoctopus/pickable/Pickable.h"
#include "godoctopus/projectile/CustomBasicProjectile.h"
#include "godoctopus/projectile/HeavyfireBotProjectile.h"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "godoctopus/production/PlayerUpgradeProduction.h"
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
		.add<octopus::Caster>(ecs.component(ProximityBeaconSpawnAbility::NAME().c_str()));
	ecs.prefab("advanced_resource")
		.auto_override<octopus::ResourceStock>()
		.auto_override<octopus::Caster>()
		.add<octopus::Caster>(ecs.component(ProximityBeaconSpawnAbility::NAME().c_str()));

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

	// Player
	flecs::entity p0 = ecs.entity()
		.set<octopus::PlayerInfo>({0, 0})
		.add<octopus::ResourceStock>()
		.add<octopus::ResourceSpent>()
		.add<PlayerUnitLoadout>()
		.add<PlayerRuneLoadout>()
		.add<octopus::PlayerUpgrade>();
	// Enemy
	flecs::entity p1 = ecs.entity()
		.set<octopus::PlayerInfo>({1, 1})
		.add<octopus::ResourceStock>()
		.add<octopus::ResourceSpent>()
		.add<PlayerUnitLoadout>()
		.add<PlayerRuneLoadout>()
		.add<octopus::PlayerUpgrade>();

	declare_prefab(ecs);
	// abilites and special units
	declare_earbot_steam_ability(ecs, game.get_step_context());
	declare_armorbot_ability(ecs, game);
	declare_heavyfire_bot_projectile_systems(ecs, game.get_world().position_context, game.get_step_context().step_manager);

	// Beacon abilities (ability spawning buildings)
	declare_basic_resource_producer_beacon_ability(ecs, game, meta_data);
	declare_advanced_resource_producer_beacon_ability(ecs, game, meta_data);
	declare_unit_producer_beacon_ability(ecs, game, meta_data);

	// Player basic buff systems
	declare_health_buff_systems(ecs);
	declare_damage_buff_systems(ecs);

	octopus::ProductionTemplateLibrary<custom_step_manager> &prod_library = ecs.get_mut<octopus::ProductionTemplateLibrary<custom_step_manager>>();
	if (meta_data.has("PlayerUpgrades")) {
		Dictionary const &prod_dict = meta_data["PlayerUpgrades"];
		Array const &prod_names = prod_dict.keys();
		for (int i = 0; i < prod_names.size(); ++i) {
			const String prod_name = prod_names[i];
			Dictionary const &prod_data = prod_dict[prod_name];
			const int64_t duration = int64_t(prod_data["duration"]) * TICK_RATE;
			std::unordered_map<std::string, octopus::Fixed> costs;
			if (prod_data.has("costs")) {
				Dictionary const &costs_dict = prod_data["costs"];
				Array const &resource_names = costs_dict.keys();
				for (int j = 0; j < resource_names.size(); ++j) {
					const String resource_name = resource_names[j];
					const octopus::Fixed cost = octopus::Fixed(double(costs_dict[resource_name]));
					costs[resource_name.utf8().get_data()] = cost;
				}
			}
			prod_library.add_template(new PlayerUpgradeProduction(prod_name.utf8().get_data(), duration, costs));
			const std::string prod_name_std = prod_name.utf8().get_data();
			ecs.prefab("base_central").add<octopus::ProductionQueue>(ecs.component(prod_name_std.c_str()));
			p0.get_mut<octopus::PlayerUpgrade>().upgrades["INTERNAL_UPGRADE_" + prod_name_std] = 1;
			p1.get_mut<octopus::PlayerUpgrade>().upgrades["INTERNAL_UPGRADE_" + prod_name_std] = 1;
		}
	}

	if (meta_data.has("PlayerProduction")) {
		Dictionary const &prod_dict = meta_data["PlayerProduction"];
		Array const &prod_names_0 = prod_dict["0"];
		octopus::PlayerProduction p0_prod;
		for (int i = 0; i < prod_names_0.size(); ++i) {
			const String prod_name = prod_names_0[i];
			p0_prod.productions[prod_name.utf8().get_data()] = true;
		}
		p0.set<octopus::PlayerProduction>(p0_prod);
		Array const &prod_names_1 = prod_dict["1"];
		octopus::PlayerProduction p1_prod;
		for (int i = 0; i < prod_names_1.size(); ++i) {
			const String prod_name = prod_names_1[i];
			p1_prod.productions[prod_name.utf8().get_data()] = true;
		}
		p1.set<octopus::PlayerProduction>(p1_prod);
	}

	// Load basic buff for e bots
	if (meta_data.has("EnemyBuffs")) {
		Dictionary const &buff_dict = meta_data["EnemyBuffs"];
		if (buff_dict.has("HealthBuff")) {
			Dictionary const &health_buff_dict = buff_dict["HealthBuff"];
			PlayerHealthBuff health_buff;
			health_buff.hp_bonus = octopus::Fixed(int(health_buff_dict["hp_bonus"]));
			p1.set<octopus::PlayerBuff<Unit, PlayerHealthBuff, octopus::HitPoint, octopus::HitPointMax>>({health_buff});
		}
		if (buff_dict.has("DamageBuff")) {
			Dictionary const &damage_buff_dict = buff_dict["DamageBuff"];
			PlayerDamageBuff damage_buff;
			damage_buff.damage_bonus = octopus::Fixed(int(damage_buff_dict["damage_bonus"]));
			p1.set<octopus::PlayerBuff<Unit, PlayerDamageBuff, octopus::Attack>>({damage_buff});
		}
	}
}

void AttackMoveDemoNode::system_setup(Dictionary const &meta_data, GameNode &game) {
	init_nodes();

	flecs::world &ecs = game.get_world().ecs;

	declare_basic_projectile_systems(ecs, _particules, _particule_orchestrator, game.get_world());
	declare_attack_particule_systems(ecs, _vat_library, _particules);
	declare_windup_projectile_systems(ecs, _vat_library, _particules);
	declare_death_particle_systems(ecs, _particules);
	declare_triggers(ecs, game.get_world().position_context, game.get_step_context().step_manager, game.get_smart_mmesh_library());
}

}
