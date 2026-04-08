#include "InfoNode.h"

#include <cmath>

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/player/Team.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "octopus/world/ability/AbilityTemplateLibrary.hh"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "godoctopus/components/types/Types.h"
#include "godoctopus/components/proximity_sensor/ProximitySensor.h"

#include "octopus_types.h"

namespace godot {

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void InfoNode::_bind_methods() {
	BIND_NODE_PATH(InfoNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &InfoNode::setup);
	ClassDB::bind_method(D_METHOD("query_stats_info", "group", "stats"), &InfoNode::query_stats_info);
	ClassDB::bind_method(D_METHOD("query_ability_castable", "group", "ability_name", "out_info"), &InfoNode::query_ability_castable);

	ADD_SIGNAL(MethodInfo("is_ready"));
	ADD_SIGNAL(MethodInfo("ability_castable_ready"));
}

void InfoNode::setup() {
	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, ecs](flecs::iter&) {
			std::lock_guard<std::mutex> lock(_mutex);

			// stats query
			{
				flecs::entity query_entity;
				for (auto e : _query_entities) {
					if (e.is_valid() && e.is_alive() && e.enabled()) {
						query_entity = e;
						break;
					}
				}
				if (_stats_info.is_valid() && query_entity.is_valid()) {
					// reset stats
					_stats_info->set_type("");
					_stats_info->set_team(0);
					_stats_info->set_hp(0.);
					_stats_info->set_hp_max(0.);
					_stats_info->set_armor(0.);
					_stats_info->set_damage(0.);
					_stats_info->set_reload_time(0.);
					_stats_info->set_special(0.);
					_stats_info->set_affinity(0.);
					_stats_info->set_proximity_sensor_activated(false);

					// query stats
					auto prefab_type = query_entity.try_get<PrefabType>();
					if (prefab_type) { _stats_info->set_type(prefab_type->name.c_str()); }
					auto team = query_entity.try_get<octopus::Team>();
					if (team) { _stats_info->set_team(team->team); }
					auto hp = query_entity.try_get<octopus::HitPoint>();
					if (hp) { _stats_info->set_hp(hp->qty.to_double()); }
					auto hp_max = query_entity.try_get<octopus::HitPointMax>();
					if (hp_max) { _stats_info->set_hp_max(hp_max->qty.to_double()); }
					auto armor = query_entity.try_get<octopus::Armor>();
					if (armor) { _stats_info->set_armor(armor->qty.to_double()); }
					auto atk = query_entity.try_get<octopus::Attack>();
					if (atk) {
						_stats_info->set_damage(atk->cst.damage.to_double());
						_stats_info->set_reload_time((double)(atk->cst.reload_time)/TICK_RATE);
					}
					auto spec = query_entity.try_get<Special>();
					if (spec) {
						_stats_info->set_special(spec->value.to_double());
						_stats_info->set_affinity(spec->affinity.to_double());
					}
					auto proximity_sensor = query_entity.try_get<ProximitySensor>();
					if (proximity_sensor) {
						_stats_info->set_proximity_sensor_activated(proximity_sensor->activated);
					}

					// tag stats as ready
					_stats_info->set_ready(true);
					this->call_deferred("emit_signal", "is_ready");
					// clear query
					_stats_info = Ref<StatsInfo>();
					_query_entities.clear();
				}
			}

			// ability castable query
			if (_ability_castable_info.is_valid() && !_ability_castable_entities.empty()) {
				flecs::entity cast_entity;
				for (auto e : _ability_castable_entities) {
					if (e.is_valid() && e.is_alive() && e.enabled()) {
						cast_entity = e;
						break;
					}
				}

				bool castable = false;
				if (cast_entity.is_valid()) {
					auto const *lib = ecs.try_get<octopus::AbilityTemplateLibrary<custom_step_manager>>();
					if (lib) {
						auto const *tmpl = lib->try_get(_ability_castable_name);
						if (tmpl) {
							std::string reason = tmpl->is_castable(cast_entity, ecs);
							castable = reason.empty();
							_ability_castable_info->set_reason(reason.c_str());
						}
					}
				}

				_ability_castable_info->set_castable(castable);
				_ability_castable_info->set_ready(true);
				this->call_deferred("emit_signal", "ability_castable_ready");
				// clear query
				_ability_castable_info = Ref<AbilityCastableInfo>();
				_ability_castable_entities.clear();
			}
		});
}

void InfoNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &InfoNode::setup));
	}
}

void InfoNode::_notification(int p_notification)
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
