#include "InfoProxyNode.h"

#include "godoctopus/components/proximity_sensor/ProximitySensor.h"
#include "godoctopus/components/special/Special.h"
#include "godoctopus/components/types/Types.h"
#include "godoctopus/components/building/Building.h"
#include "godoctopus/pickable/Pickable.h"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/world/player/PlayerInfo.hh"

namespace godot {

InfoProxyNodeDataLocker::InfoProxyNodeDataLocker(InfoProxyNode *node_p, std::mutex &mutex_p) :
	proxy_map(node_p->_proxy_map), node(node_p), lock(mutex_p) {}

bool InfoProxyNodeDataLocker::is_up_to_date(EntityGroup *group) const {
	// Check if the group's timestamp is up to date
	return group->get_timestamp() != -1 && node->_last_refresh_time > group->get_timestamp();
}
// Will be called by Godot when the class is registered
// Use this to add properties to your class
void InfoProxyNode::_bind_methods() {
	BIND_NODE_PATH(InfoProxyNode, GameNode, game_node);
	ADD_SIMPLE_PROP(InfoProxyNode, INT, refresh_tick);

	ClassDB::bind_method(D_METHOD("setup"), &InfoProxyNode::setup);
	ClassDB::bind_method(D_METHOD("get_proxy_from_group", "group"), &InfoProxyNode::get_proxy_from_group);
	ClassDB::bind_method(D_METHOD("get_target_from_group", "group"), &InfoProxyNode::get_target_from_group);
}

TypedArray<Ref<InfoProxyResource>> InfoProxyNode::get_proxy_from_group(Ref<EntityGroup> group) const {
	TypedArray<Ref<InfoProxyResource>> data_array;
	if (!group.is_valid()) {
		return data_array;
	}
	std::lock_guard<std::mutex> lock(_mutex);
	for (flecs::entity e : group->get_entities()) {
		auto it = _proxy_map.find(e.id());
		if (it != _proxy_map.end()) {
			data_array.append(it->second.duplicate());
		}
	}
	return data_array;
}

static void insert_if_unique(std::vector<octopus::Vector> &targets, octopus::Vector const &new_target) {
	for (const auto &target : targets) {
		if (octopus::square_length(target -new_target) < 0.1) {
			return;
		}
	}
	targets.push_back(new_target);
}

TypedArray<Ref<InfoTargetResource>> InfoProxyNode::get_target_from_group(Ref<EntityGroup> group) const {
	TypedArray<Ref<InfoTargetResource>> data_array;
	if (!group.is_valid()) {
		return data_array;
	}
	std::lock_guard<std::mutex> lock(_mutex);
	std::vector<octopus::Vector> move_targets;
	std::vector<octopus::Vector> attack_targets;

	// Build unique target list
	for (flecs::entity e : group->get_entities()) {
		auto it = _proxy_map.find(e.id());
		if (it != _proxy_map.end()) {
			const bool has_attack_target = it->second.get_has_attack_target();
			const bool has_move_target = it->second.get_has_move_target();
			// only if target
			if (has_attack_target || has_move_target) {
				octopus::Vector const &target = it->second.get_target();
				if (has_attack_target) {
					insert_if_unique(attack_targets, target);
				} else if (has_move_target) {
					insert_if_unique(move_targets, target);
				}
			}
		}
	}

	auto convert_to_resource = [](octopus::Vector const &target, bool is_attack) {
		Ref<InfoTargetResource> target_resource = Ref<InfoTargetResource>(memnew(InfoTargetResource));
		target_resource->set_position({(float)target.x.to_double(), (float)target.y.to_double()});
		target_resource->set_is_attack(is_attack);
		return target_resource;
	};

	// Convert data to Godot resources
	for (const auto &target : move_targets) {
		data_array.append(convert_to_resource(target, false));
	}
	for (const auto &target : attack_targets) {
		data_array.append(convert_to_resource(target, true));
	}
	return data_array;
}

void InfoProxyNode::setup() {
	using namespace octopus;

	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;

	// Create Position, Velocity query that matches empty archetypes.
	flecs::query<Position, PrefabType*, PlayerAppartenance*, Team*, HitPoint*, HitPointMax*, Armor*, Attack*, Special*, ProximitySensor*, MoveCommand*, AttackCommand*, PickableSetUp*> update_query =
		ecs.query<Position, PrefabType*, PlayerAppartenance*, Team*, HitPoint*, HitPointMax*, Armor*, Attack*, Special*, ProximitySensor*, MoveCommand*, AttackCommand*, PickableSetUp*>();

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, ecs, update_query](flecs::iter&) {
			if (refresh_tick > 0 && octopus::get_time_stamp(ecs) % refresh_tick != 0) {
				return;
			}
			std::lock_guard<std::mutex> lock(_mutex);

			_last_refresh_time = octopus::get_time_stamp(ecs);

			// reset proxy map to avoid keeping data of entities that are no longer valid
			_proxy_map.clear();

			update_query.each([this](flecs::entity e,
				Position &pos,
				PrefabType *prefab_type,
				PlayerAppartenance *player_appartenance,
				Team *team,
				HitPoint *hp,
				HitPointMax *hp_max,
				Armor *armor,
				Attack *atk,
				Special *spec,
				ProximitySensor *proximity_sensor,
				MoveCommand *move_cmd,
				AttackCommand *atk_cmd,
				PickableSetUp *pickable)
			{
				InfoProxyData &infos_data = _proxy_map[e.id()];

				infos_data.set_position(pos.pos);
				infos_data.set_has_attack_target(atk_cmd);
				infos_data.set_has_move_target(move_cmd);
				if (atk_cmd) {
					infos_data.set_target(atk_cmd->target_pos);
				}
				else if (move_cmd) {
					infos_data.set_target(move_cmd->target);
				}

				infos_data.set_building(e.has<Building>());
				if (prefab_type) { infos_data.set_type(prefab_type->name.c_str()); }
				if (team) { infos_data.set_team(team->team); }
				if (player_appartenance) { infos_data.set_player(player_appartenance->idx); }
				if (hp) { infos_data.set_hp(hp->qty.to_double()); }
				if (hp_max) { infos_data.set_hp_max(hp_max->qty.to_double()); }
				if (armor) { infos_data.set_armor(armor->qty.to_double()); }
				if (atk) {
					infos_data.set_damage(atk->cst.damage.to_double());
					infos_data.set_reload_time((double)(atk->cst.reload_time)/TICK_RATE);
				}
				if (spec) {
					infos_data.set_special(spec->value.to_double());
					infos_data.set_affinity(spec->affinity.to_double());
				}
				if (proximity_sensor) {
					infos_data.set_proximity_sensor_activated(proximity_sensor->activated);
				}
				if (pickable) {
					infos_data.set_pickable_id(pickable->id);
				} else {
					infos_data.set_pickable_id(-1);
				}
				infos_data.set_alive(e.is_alive() && e.enabled());
			});
		});
}

void InfoProxyNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &InfoProxyNode::setup));
	}
}

void InfoProxyNode::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
			init_nodes();
		} break;
	}
}

}
