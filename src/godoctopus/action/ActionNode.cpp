#include "ActionNode.h"

#include <cmath>
#include <set>

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/position/PositionInTree.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/triangulation/DelaunayTriangulation.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "godoctopus/components/Dummy.h"
#include "godoctopus/components/building/Building.h"
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
	ClassDB::bind_method(D_METHOD("place_building", "prefab", "position", "team"), &ActionNode::place_building);
	ClassDB::bind_method(D_METHOD("snap_building_position", "position"), &ActionNode::snap_building_position);
	ClassDB::bind_method(D_METHOD("get_building_occupied_cells", "min_cell", "max_cell"), &ActionNode::get_building_occupied_cells);
	ClassDB::bind_method(D_METHOD("get_building_placement_preview", "prefab", "position"), &ActionNode::get_building_placement_preview);

	ADD_SIGNAL(MethodInfo("building_placement_finished",
		PropertyInfo(Variant::INT, "request_id"),
		PropertyInfo(Variant::BOOL, "success"),
		PropertyInfo(Variant::VECTOR2, "position"),
		PropertyInfo(Variant::STRING, "reason")));
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
	data.stats_set_coef.values[godoctopus::StatsType::HitPoints] = octopus::Fixed(int(rune_data.get("HitPointsCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::Damage] = octopus::Fixed(int(rune_data.get("DamageCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::Shield] = octopus::Fixed(int(rune_data.get("ShieldCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::MechanicalPower] = octopus::Fixed(int(rune_data.get("MechanicalPowerCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::MechanicalArmor] = octopus::Fixed(int(rune_data.get("MechanicalArmorCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::PlasmaPower] = octopus::Fixed(int(rune_data.get("PlasmaPowerCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::PlasmaArmor] = octopus::Fixed(int(rune_data.get("PlasmaArmorCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::Speed] = octopus::Fixed(int(rune_data.get("SpeedCoefPercent", 0)))/100;
	data.stats_set_coef.values[godoctopus::StatsType::Affinity] = octopus::Fixed(int(rune_data.get("AffinityCoefPercent", 0)))/100;
	const String stats_type_str = rune_data.get("StatsType", "Affinity");
	data.stats_type = godoctopus::type_from_string(stats_type_str.utf8().get_data());
	data.modifier_priority = rune_data.get("stats_modifier_priority", 0);
	return data;
}

void ActionNode::mod_rune(String const &unit_type, String const &rune_type, int player_idx, Dictionary rune_data, bool add) {
	std::lock_guard<std::mutex> lock(_mutex);
	_actions.push_back(ModRuneAction{unit_type, rune_type, player_idx, createRuneData(rune_data), add});
}

int64_t ActionNode::place_building(String const &prefab, Vector2 const &position, int team) {
	ERR_FAIL_NULL_V(_game_node, -1);
	int64_t const request_id = _next_placement_request_id.fetch_add(1);
	std::lock_guard<std::mutex> lock(_mutex);
	_actions.push_back(PlaceBuildingAction{prefab, position, team, request_id});
	return request_id;
}

void ActionNode::emit_placement_result(int64_t request_id, bool success, Vector2 const &position, String const &reason) {
	call_deferred("emit_signal", "building_placement_finished", request_id, success, position, reason);
}

Vector2i ActionNode::snap_building_position(Vector2 const &position) const {
	if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
		return Vector2i();
	}
	return Vector2i(
		static_cast<int32_t>(BuildingFootprintManager::snap_to_grid(octopus::Fixed(position.x))),
		static_cast<int32_t>(BuildingFootprintManager::snap_to_grid(octopus::Fixed(position.y)))
	);
}

Array ActionNode::get_building_occupied_cells(Vector2i const &min_cell, Vector2i const &max_cell) const {
	Array cells;
	ERR_FAIL_NULL_V(_game_node, cells);
	if (min_cell.x > max_cell.x || min_cell.y > max_cell.y) {
		return cells;
	}

	std::set<BuildingFootprintManager::GridCell> occupied_cells;
	{
		std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
		flecs::world &ecs = _game_node->get_world().ecs;
		flecs::query<Building const, octopus::Position const, octopus::Collision const> building_query =
			ecs.query<Building const, octopus::Position const, octopus::Collision const>();
		building_query.each([&](flecs::entity, Building const &, octopus::Position const &position, octopus::Collision const &collision) {
			for (BuildingFootprintManager::GridCell const &cell : BuildingFootprintManager::get_grid_cells(
				position.pos.x,
				position.pos.y,
				collision.ray)) {
				if (cell.first >= min_cell.x && cell.first <= max_cell.x &&
					cell.second >= min_cell.y && cell.second <= max_cell.y) {
					occupied_cells.insert(cell);
				}
			}
		});
		std::set<BuildingFootprintManager::GridCell> const pending_cells =
			_building_footprint_manager.get_pending_cells(min_cell.x, min_cell.y, max_cell.x, max_cell.y);
		occupied_cells.insert(pending_cells.begin(), pending_cells.end());
	}

	for (BuildingFootprintManager::GridCell const &cell : occupied_cells) {
		cells.push_back(Vector2i(static_cast<int32_t>(cell.first), static_cast<int32_t>(cell.second)));
	}
	return cells;
}

Dictionary ActionNode::get_building_placement_preview(String const &prefab_name, Vector2 const &position) const {
	Dictionary preview;
	preview["valid"] = false;
	preview["reason"] = "invalid position";
	preview["position"] = position;
	preview["cells"] = Array();
	preview["radius"] = 0.0;
	if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
		return preview;
	}
	ERR_FAIL_NULL_V(_game_node, preview);
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());

	Ref<UnitPrefab> prefab = _game_node->get_prefab(prefab_name);
	if (!prefab.is_valid()) {
		preview["reason"] = "unknown prefab";
		return preview;
	}
	if (!prefab->get_is_building()) {
		preview["reason"] = "prefab is not a building";
		return preview;
	}
	int const ray_x100 = prefab->get_ray_x100();
	if (ray_x100 < 2) {
		preview["reason"] = "building footprint is too small";
		return preview;
	}

	Vector2i const snapped = snap_building_position(position);
	Vector2 const snapped_position(snapped.x, snapped.y);
	octopus::Fixed const radius = octopus::Fixed(ray_x100) / 100;
	std::vector<BuildingFootprintManager::GridCell> const requested_cells =
		BuildingFootprintManager::get_grid_cells(
			octopus::Fixed(snapped.x),
			octopus::Fixed(snapped.y),
			radius);
	Array cells;
	for (BuildingFootprintManager::GridCell const &cell : requested_cells) {
		cells.push_back(Vector2i(static_cast<int32_t>(cell.first), static_cast<int32_t>(cell.second)));
	}
	preview["position"] = snapped_position;
	preview["cells"] = cells;
	preview["radius"] = static_cast<double>(ray_x100) / 100.0;

	flecs::world &ecs = _game_node->get_world().ecs;
	flecs::query<Building const, octopus::Position const, octopus::Collision const> building_query =
		ecs.query<Building const, octopus::Position const, octopus::Collision const>();
	std::set<BuildingFootprintManager::GridCell> occupied_cells;
	building_query.each([&occupied_cells](
		flecs::entity,
		Building const &,
		octopus::Position const &building_position,
		octopus::Collision const &collision
	) {
		for (BuildingFootprintManager::GridCell const &cell : BuildingFootprintManager::get_grid_cells(
			building_position.pos.x,
			building_position.pos.y,
			collision.ray)) {
			occupied_cells.insert(cell);
		}
	});
	bool const valid = !_building_footprint_manager.overlaps(requested_cells, occupied_cells);
	preview["valid"] = valid;
	preview["reason"] = valid ? String() : String("building footprint cells are occupied");
	return preview;
}

void ActionNode::setup() {
	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;
	auto& step_manager = _game_node->get_step_context().step_manager;
	flecs::query<octopus::PlayerInfo> query_player = ecs.query<octopus::PlayerInfo>();
	flecs::query<Building const, octopus::Position const, octopus::Collision const> building_query =
		ecs.query<Building const, octopus::Position const, octopus::Collision const>();
	ecs.component<RuntimePlacedBuilding>();
	ecs.observer<RuntimePlacedBuilding>()
		.event(flecs::OnRemove)
	.each([this](flecs::entity entity, RuntimePlacedBuilding const &) {
		if (_game_node) {
			_building_footprint_manager.remove_building(entity.id(), _game_node->get_delaunay_triangulation());
		}
	});
	// flecs::query<RuneLoad<DefaultRune>> query_rune = ecs.query<RuneLoad<DefaultRune>>();

	ecs.system<>()
		.immediate()
		.kind(ecs.entity(InputPhase))
		.run([this, ecs, &step_manager, query_player, building_query](flecs::iter&) {
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
				else if (std::holds_alternative<PlaceBuildingAction>(action)) {
					PlaceBuildingAction const &place_action = std::get<PlaceBuildingAction>(action);
					Vector2 result_position = place_action.position;
					auto reject = [this, &place_action, &result_position](String const &reason) {
						emit_placement_result(place_action.request_id, false, result_position, reason);
					};

					if (!std::isfinite(place_action.position.x) || !std::isfinite(place_action.position.y)) {
						reject("invalid position");
						continue;
					}
					Ref<UnitPrefab> prefab = _game_node->get_prefab(place_action.prefab);
					if (!prefab.is_valid()) {
						reject("unknown prefab");
						continue;
					}
					if (!prefab->get_is_building()) {
						reject("prefab is not a building");
						continue;
					}
					int const ray_x100 = prefab->get_ray_x100();
					if (ray_x100 < 2) {
						reject("building footprint is too small");
						continue;
					}

					int64_t const grid_x = BuildingFootprintManager::snap_to_grid(octopus::Fixed(place_action.position.x));
					int64_t const grid_y = BuildingFootprintManager::snap_to_grid(octopus::Fixed(place_action.position.y));
					result_position = Vector2(static_cast<double>(grid_x), static_cast<double>(grid_y));
					octopus::Fixed const center_x(grid_x);
					octopus::Fixed const center_y(grid_y);
					octopus::Fixed const radius = octopus::Fixed(ray_x100) / 100;
					std::vector<BuildingFootprintManager::GridCell> const requested_cells =
						BuildingFootprintManager::get_grid_cells(center_x, center_y, radius);

					std::set<BuildingFootprintManager::GridCell> occupied_cells;
					building_query.each([&occupied_cells](
						flecs::entity,
						Building const &,
						octopus::Position const &position,
						octopus::Collision const &collision
					) {
						for (BuildingFootprintManager::GridCell const &cell : BuildingFootprintManager::get_grid_cells(
							position.pos.x,
							position.pos.y,
							collision.ray)) {
							occupied_cells.insert(cell);
						}
					});
					if (_building_footprint_manager.overlaps(requested_cells, occupied_cells)) {
						reject("building footprint cells are occupied");
						continue;
					}

					octopus::StepEntityManager *entity_manager = ecs.try_get_mut<octopus::StepEntityManager>();
					if (!entity_manager) {
						reject("entity creation manager is unavailable");
						continue;
					}
					_building_footprint_manager.reserve_cells(place_action.request_id, requested_cells);
					std::string const type = place_action.prefab.utf8().get_data();
					octopus::Position position;
					position.pos.x = center_x;
					position.pos.y = center_y;
					octopus::EntityCreationStep step;
					step.set_up_function = [
						this,
						ecs,
						position,
						type,
						place_action,
						result_position,
						ray_x100
					](flecs::entity new_ent, flecs::world const &) {
						custom_queue queue;
						new_ent
							.set<octopus::Position>(position)
							.is_a(ecs.prefab(type.c_str()))
							.set<octopus::Team>({static_cast<uint16_t>(place_action.team)})
							.set<octopus::PlayerAppartenance>({static_cast<uint32_t>(place_action.team)})
							.set<custom_queue>(queue)
							.add<RuntimePlacedBuilding>();
						_building_footprint_manager.add_building(
							new_ent,
							result_position,
							ray_x100,
							_game_node->get_delaunay_triangulation());
						_building_footprint_manager.release_cells(place_action.request_id);
					};
					entity_manager->get_last_layer().push_back(step);
					emit_placement_result(place_action.request_id, true, result_position, String());
				}
				else if (std::holds_alternative<ModRuneAction>(action)) {
					ModRuneAction const &mod_rune_action = std::get<ModRuneAction>(action);
					std::string unit_type = mod_rune_action.unit_type.utf8().get_data();
					std::string rune_type = mod_rune_action.rune_type.utf8().get_data();
					uint32_t player_idx = static_cast<uint32_t>(mod_rune_action.player_idx);
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
				} else if (std::holds_alternative<KillUnitAction>(action)) {
					KillUnitAction const &kill_action = std::get<KillUnitAction>(action);
					if (kill_action.group.is_valid()) {
						for (flecs::entity e : kill_action.group->get_entities()) {
							if (e.is_valid() && e.is_alive()) {
								if (e.has<Dummy>()) {
									e.destruct();
								} else if (e.has<octopus::HitPointMax>()) {
									const auto& hp_max = e.get<octopus::HitPointMax>();
									auto& layer = step_manager.get_last_layer().back().template get<octopus::HitPointStep>();
									layer.add_step(e, { -10 * hp_max.qty });
								}
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
