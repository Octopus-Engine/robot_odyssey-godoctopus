#include "ActionNode.h"

#include <cmath>

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/position/PositionInTree.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "godoctopus/components/Dummy.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"

namespace godot {

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void ActionNode::_bind_methods() {
	BIND_NODE_PATH(ActionNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &ActionNode::setup);
	ClassDB::bind_method(D_METHOD("spawn_units", "prefab", "position", "team", "count"), &ActionNode::spawn_units);
	ClassDB::bind_method(D_METHOD("spawn_units_attack_move", "prefab", "position", "team", "count", "target"), &ActionNode::spawn_units_attack_move);
	ClassDB::bind_method(D_METHOD("spawn_units_in_group", "prefab", "position", "team", "count", "group"), &ActionNode::spawn_units_in_group);
	ClassDB::bind_method(D_METHOD("spawn_dummy_units_in_group", "prefab", "position", "team", "count", "group"), &ActionNode::spawn_dummy_units_in_group);
	ClassDB::bind_method(D_METHOD("spawn_units_attack_move_in_group", "prefab", "position", "team", "count", "target", "group"), &ActionNode::spawn_units_attack_move_in_group);
	ClassDB::bind_method(D_METHOD("mod_hp", "group", "hp_delta"), &ActionNode::mod_hp);
	ClassDB::bind_method(D_METHOD("mod_rune", "unit_type", "rune_type", "player_idx", "rune_data", "add"), &ActionNode::mod_rune);
	ClassDB::bind_method(D_METHOD("spawn_prop", "position", "ray_x100"), &ActionNode::spawn_prop);
}

static ModRuneData createRuneData(Dictionary rune_data) {
	ModRuneData data;
	data.level = rune_data.get("level", 0);
	data.flat_buff = rune_data.get("flat_buff", 0);
	data.base = rune_data.get("base", 0);
	data.level_upgrade = rune_data.get("level_upgrade", 0);
	data.upgrade = rune_data.get("upgrade", 0);
	data.range = rune_data.get("range", 0);
	data.duration_ticks = rune_data.get("duration_ticks", 0);
	return data;
}

void ActionNode::mod_rune(String const &unit_type, String const &rune_type, int player_idx, Dictionary rune_data, bool add) {
	std::lock_guard<std::mutex> lock(_mutex);
	_actions.push_back(ModRuneAction{unit_type, rune_type, player_idx, createRuneData(rune_data), add});
}

void ActionNode::setup() {
	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;
	auto& step_manager = _game_node->get_step_context().step_manager;
	flecs::query<octopus::PlayerInfo> query_player = ecs.query<octopus::PlayerInfo>();
	// flecs::query<RuneLoad<DefaultRune>> query_rune = ecs.query<RuneLoad<DefaultRune>>();

	ecs.system<>()
		.immediate()
		.kind(ecs.entity(InputPhase))
		.run([this, ecs, &step_manager, query_player](flecs::iter&) {
			// handle actions
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto action : _actions) {
				if (std::holds_alternative<SpawnUnitsAction>(action)) {
					SpawnUnitsAction const &spawn_action = std::get<SpawnUnitsAction>(action);
					std::string type = spawn_action.prefab.utf8().get_data();
					int number = spawn_action.count;
					double x = spawn_action.position.x;
					double y = spawn_action.position.y;

					int ray = number == 1?0:1;
					int start_circle = 0;
					int cur_circle = 3;
					for(int i = 0 ; i < number ; ++ i)
					{
						if(i >= start_circle + cur_circle)
						{
							ray += 2;
							start_circle = i;
							cur_circle = 3*ray;
						}
						octopus::Position pos_l;
						pos_l.pos.x = x + std::cos(2*3.14*i/cur_circle) * ray;
						pos_l.pos.y = y + std::sin(2*3.14*i/cur_circle) * ray;

						octopus::EntityCreationStep step_l;
						step_l.set_up_function = [ecs, pos_l, type, spawn_action](flecs::entity new_ent, flecs::world const &world_p) {
							octopus::Logger::getDebug() << "producing"<<std::endl;
							custom_queue queue_l;

							if(spawn_action.attack_move)
							{
								octopus::AttackCommand atk_l {flecs::entity(), {spawn_action.attack_move_target.x, spawn_action.attack_move_target.y}, true};
								queue_l._queuedActions.push_back(octopus::CommandQueueActionAddBack<custom_variant> {atk_l});
							}

							new_ent.set<octopus::Position>(pos_l)
								.is_a(ecs.prefab(type.c_str()))
								.set<octopus::Team>({(uint16_t)spawn_action.team})
								.set<octopus::PlayerAppartenance>({(uint32_t)spawn_action.team})
								.set<custom_queue>(queue_l)
							;
							// update group if available
							if (spawn_action.group.is_valid()) {
								spawn_action.group->increase_populated(octopus::get_time_stamp(ecs));
								spawn_action.group->get_entities().push_back(new_ent);
							}

							if (spawn_action.dummy) {
								new_ent.add<Dummy>()
									   .remove<octopus::PositionInTree>()
									   .remove<Pickable>()
									   .remove<VatLibraryHandle>()
									   .remove<octopus::Collision>()
									   .remove<custom_queue>();
							}
						};

						octopus::Logger::getDebug() << "adding ent creation"<<std::endl;
						ecs.try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step_l);
					}
				}
				else if (std::holds_alternative<ModRuneAction>(action)) {
					ModRuneAction const &mod_rune_action = std::get<ModRuneAction>(action);
					std::string unit_type = mod_rune_action.unit_type.utf8().get_data();
					std::string rune_type = mod_rune_action.rune_type.utf8().get_data();
					int player_idx = mod_rune_action.player_idx;
					bool add = mod_rune_action.add;

					// get player info
					flecs::entity player = query_player.find([&player_idx](octopus::PlayerInfo& p) {
						return p.idx == player_idx;
					});

					octopus::Logger::getDebug() << "Modding rune "<< (add ? "add" : "remove") <<" for player: "<<player_idx<<", unit_type: "<<unit_type<<", rune_type: "<<rune_type<<std::endl;
					octopus::Logger::getDebug() << "\tLevel: "<<mod_rune_action.rune_data.level<<", flat_buff: "<<mod_rune_action.rune_data.flat_buff
												<<", base: "<<mod_rune_action.rune_data.base<<", upgrade: "<<mod_rune_action.rune_data.upgrade
												<<", range: "<<mod_rune_action.rune_data.range<<", duration_ticks: "<<mod_rune_action.rune_data.duration_ticks<<std::endl;
					mod_rune_based_on_names(player, unit_type, rune_type, add, mod_rune_action.rune_data);
				}
				else if (std::holds_alternative<SpawnPropAction>(action)) {
					SpawnPropAction const &prop_action = std::get<SpawnPropAction>(action);

					octopus::EntityCreationStep step_l;
					step_l.set_up_function = [prop_action](flecs::entity new_ent, flecs::world const &) {
						octopus::Position pos_l;
						pos_l.pos.x = prop_action.position.x;
						pos_l.pos.y = prop_action.position.y;
						octopus::Collision col_l;
						col_l.ray = octopus::Fixed(prop_action.ray_x100) / 100;
						col_l.mass = octopus::Fixed(1000);
						new_ent
							.set<octopus::Position>(pos_l)
							.set<octopus::Collision>(col_l)
							.add<octopus::PositionInTree>()
						;
					};
					octopus::Logger::getDebug() << "adding prop creation"<<std::endl;
					ecs.try_get_mut<octopus::StepEntityManager>()->get_last_layer().push_back(step_l);
				} else if (std::holds_alternative<HpModification>(action)) {
					HpModification const &hp_mod_action = std::get<HpModification>(action);
					if (hp_mod_action.group.is_valid()) {
						for (flecs::entity e : hp_mod_action.group->get_entities()) {
							if (e.is_valid() && e.is_alive()) {
								auto& layer = step_manager.get_last_layer().back().template get<octopus::HitPointStep>();
								layer.add_step(e, {hp_mod_action.hp_delta});
							}
						}
					}
				}
			}
			_actions.clear();
		});
}

void ActionNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &ActionNode::setup));
	}
}

void ActionNode::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
		} break;
	}
}

}
