#include "InfoNode.h"

#include <cmath>

#include "octopus/components/basic/position/Position.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/world/step/StepEntityManager.hh"
#include "godoctopus/trigger_module/TriggerDeclaration.h"
#include "godoctopus/components/Explorator.h"

namespace godot {

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void InfoNode::_bind_methods() {
	BIND_NODE_PATH(InfoNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("setup"), &InfoNode::setup);
	ClassDB::bind_method(D_METHOD("query_stats_info", "group", "stats"), &InfoNode::query_stats_info);
	ClassDB::bind_method(D_METHOD("get_unit_vision_data"), &InfoNode::get_unit_vision_data);
}

void InfoNode::setup() {
	if(!_game_node) {
		return;
	}
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world& ecs = _game_node->get_world().ecs;

	flecs::query<octopus::Position, Explorator> vision_query = ecs.query<octopus::Position, Explorator>();

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.run([this, ecs, vision_query](flecs::iter&) {
			std::lock_guard<std::mutex> lock(_mutex);

			_unit_vision_data.clear();
			vision_query.each([this](flecs::entity e, octopus::Position const &pos, Explorator const &expl) {
				_unit_vision_data.push_back(WORLD_SCALE * real_t(octopus::to_double(pos.pos.x)) + 500.);
				_unit_vision_data.push_back(WORLD_SCALE * real_t(octopus::to_double(pos.pos.y)) + 500.);
				_unit_vision_data.push_back(expl.visibility_range);
			});

			flecs::entity query_entity;
			for (auto e : _query_entities) {
				if (e.is_valid() && e.is_alive() && e.enabled()) {
					query_entity = e;
					break;
				}
			}

			if (!_stats_info.is_valid() || !query_entity.is_valid()) {
				return;
			}

			// reset stats
			_stats_info->set_hp(0.);
			_stats_info->set_hp_max(0.);
			_stats_info->set_armor(0.);
			_stats_info->set_damage(0.);
			_stats_info->set_reload_time(0.);
			_stats_info->set_special(0.);
			_stats_info->set_affinity(0.);

			// query stats
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

			// tag stats as ready
			_stats_info->set_ready(true);
			// clear query
			_stats_info = Ref<StatsInfo>();
			_query_entities.clear();
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
