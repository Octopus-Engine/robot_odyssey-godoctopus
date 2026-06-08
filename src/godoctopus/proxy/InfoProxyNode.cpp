#include "InfoProxyNode.h"

#include "godoctopus/components/proximity_sensor/ProximitySensor.h"
#include "godoctopus/components/special/Special.h"
#include "godoctopus/components/types/Types.h"
#include "godoctopus/components/building/Building.h"
#include "godoctopus/components/rune_load/RuneLoad.h"
#include "godoctopus/pickable/Pickable.h"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/ability/Caster.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/advanced/production/queue/ProductionQueue.hh"
#include "octopus/components/advanced/production/PlayerProduction.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/production/ProductionTemplateLibrary.hh"
#include "octopus/world/production/ProductionTemplate.hh"

#include "octopus_types.h"
#include <unordered_set>

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
	ClassDB::bind_method(D_METHOD("get_production_queue_from_group", "group"), &InfoProxyNode::get_production_queue_from_group);
	ClassDB::bind_method(D_METHOD("get_available_actions_from_group", "group"), &InfoProxyNode::get_available_actions_from_group);
	ClassDB::bind_method(D_METHOD("get_production_queue_from_player", "player_id"), &InfoProxyNode::get_production_queue_from_player);
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

TypedArray<Ref<InfoProductionQueueResource>> InfoProxyNode::get_production_queue_from_group(Ref<EntityGroup> group) const {
	TypedArray<Ref<InfoProductionQueueResource>> result;
	if (!group.is_valid()) {
		return result;
	}
	std::lock_guard<std::mutex> lock(_mutex);
	for (flecs::entity_t entity : group->get_entities()) {
		auto it = _proxy_map.find(entity);
		if (it != _proxy_map.end()) {
			const InfoProxyData &d = it->second;
			if (d.get_alive()) {
				result.append(d.get_production_queue());
			}
		}
	}
	return result;
}

TypedArray<Ref<InfoAvailableActionResource>> InfoProxyNode::get_available_actions_from_group(Ref<EntityGroup> group) const {
	TypedArray<Ref<InfoAvailableActionResource>> result;
	if (!group.is_valid()) {
		return result;
	}

	std::lock_guard<std::mutex> lock(_mutex);
	std::unordered_set<std::string> seen_action_keys;

	auto append_action = [&result, &seen_action_keys](InfoAvailableActionResource::ActionType action_type, const char *action_name, flecs::entity source_entity) {
		if (!action_name || action_name[0] == '\0') {
			return;
		}
		const std::string key = std::to_string((int)action_type) + ":" + action_name;
		if (seen_action_keys.find(key) != seen_action_keys.end()) {
			return;
		}
		seen_action_keys.insert(key);

		Ref<InfoAvailableActionResource> action_resource(memnew(InfoAvailableActionResource));
		action_resource->set_action_name(String(action_name));
		action_resource->set_action_type((int)action_type);
		action_resource->set_source_entity_id((int64_t)source_entity.id());
		result.append(action_resource);
	};

	for (flecs::entity entity : group->get_entities()) {
		if (!entity.is_valid()) {
			continue;
		}
		auto info_it = _proxy_map.find(entity.id());
		if (info_it == _proxy_map.end() || !info_it->second.get_alive()) {
			continue;
		}

		entity.each<octopus::Caster>([&append_action, entity](flecs::entity ability) {
			append_action(InfoAvailableActionResource::ACTION_CAST, ability.name().c_str(), entity);
		});
		entity.each<octopus::ProductionQueue>([&append_action, entity](flecs::entity production) {
			// Only append the production if player enables it
			if (octopus::satisfy_player_production_requirements(entity, production.name().c_str())) {
				append_action(InfoAvailableActionResource::ACTION_PRODUCTION, production.name().c_str(), entity);
			}
		});
	}

	return result;
}

TypedArray<Ref<InfoProductionQueueResource>> InfoProxyNode::get_production_queue_from_player(int player_id) const {
	TypedArray<Ref<InfoProductionQueueResource>> result;
	std::lock_guard<std::mutex> lock(_mutex);
	if (player_id < 0 || _player_to_production_entities.find((uint32_t)player_id) == _player_to_production_entities.end()) {
		return result;
	}
	uint64_t idx = 0;
	bool any_valid = true;
	while (any_valid) {
		any_valid = false;
		for (const auto &entity : _player_to_production_entities.at((uint32_t)player_id)) {
			auto it = _proxy_map.find(entity);
			if (it != _proxy_map.end()) {
				const InfoProxyData &d = it->second;
				if (d.get_alive() && idx < d.get_production_queue().size()) {
					result.append(d.get_production_queue()[idx]);
					any_valid = true;
				}
			}
		}
		idx++;
	}
	return result;
}

void InfoProxyNode::setup() {
	using namespace octopus;

	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;

	// Create Position, Velocity query that matches empty archetypes.
	flecs::query<Position, PrefabType*, PlayerAppartenance*, Team*, HitPoint*, HitPointMax*, Armor*, Attack*, Special*, ProximitySensor*, MoveCommand*, AttackCommand*, PickableSetUp*, RuneLoad<DefaultRune>*, ProductionQueue*> update_query =
		ecs.query<Position, PrefabType*, PlayerAppartenance*, Team*, HitPoint*, HitPointMax*, Armor*, Attack*, Special*, ProximitySensor*, MoveCommand*, AttackCommand*, PickableSetUp*, RuneLoad<DefaultRune>*, ProductionQueue*>();

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
			_player_to_production_entities.clear();

			update_query.each([this, ecs](flecs::entity e,
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
				PickableSetUp *pickable,
				RuneLoad<DefaultRune> *rune_load,
				ProductionQueue *production_queue)
			{
				InfoProxyData &infos_data = _proxy_map[e.id()];
				infos_data.entity = e;

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
				if (rune_load) {
					infos_data.set_rune_loads(rune_load->qty);
				} else {
					infos_data.set_rune_loads(0);
				}

				if (production_queue) {
					const auto &production_library = ecs.get<octopus::ProductionTemplateLibrary<custom_step_manager>>();
					TypedArray<Ref<InfoProductionQueueResource>> prod_array;
					bool first = true;
					for (const std::string &item : production_queue->queue) {
						Ref<InfoProductionQueueResource> prod_res = Ref<InfoProductionQueueResource>(memnew(InfoProductionQueueResource));
						prod_res->set_prod_name(item.c_str());
						if (first) {
							octopus::ProductionTemplate<custom_step_manager> const * prod_template = production_library.try_get(item);
							if (prod_template) {
								prod_res->set_progress((double)(octopus::get_time_stamp(e.world()) - production_queue->start_timestamp) / double(prod_template->duration()));
							} else {
								prod_res->set_progress(0);
							}
							first = false;
						} else {
							prod_res->set_progress(0);
						}
						prod_array.append(prod_res);
					}
					if (prod_array.size() > 0 && player_appartenance) {
						_player_to_production_entities[player_appartenance->idx].push_back(e.id());
					}
					infos_data.set_production_queue(prod_array);
				}
				else {
					infos_data.set_production_queue(TypedArray<Ref<InfoProductionQueueResource>>());
				}
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
