#include "SelectionGroup.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"

#include "flecs.h"

namespace godot {

void SelectionGroup::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
			if (_game_node) {
				_game_node->connect("init_done", callable_mp(this, &SelectionGroup::setup));
			}
		} break;
	}
}

static void update_selected(flecs::entity &e, bool selected) {
	if (!e.is_valid() || !e.is_alive() || !e.enabled()) {
		return;
	}
	auto sel = e.try_get_mut<Selected>();
	if (sel) {
		sel->selected = selected;
	}
}

static void insert_if_not_present(std::vector<flecs::entity> &entities, flecs::entity const &e) {
	if (!e.is_valid() || !e.is_alive() || !e.enabled()) {
		return;
	}
	if (std::find(entities.begin(), entities.end(), e) == entities.end()) {
		entities.push_back(e);
	}
}

void SelectionGroup::setup() {
	std::lock_guard<std::mutex> lock_progress(_game_node->get_progress_mutex());
	flecs::world &ecs = _game_node->get_world().ecs;

	ecs.system<>()
		.kind(ecs.entity(DisplaySyncPhase))
		.write<Selected>()
		.run([this](flecs::iter&) {
			std::lock_guard<std::mutex> lock(_mutex);
			if (_update_type == UpdateType::ADD) {
				auto &dst_entities = _group->get_entities();
				for(flecs::entity &e : _new_group->get_entities()) {
					update_selected(e, true);
					insert_if_not_present(dst_entities, e);
				}
			}
			else if (_update_type == UpdateType::REMOVE) {
				auto &dst_entities = _group->get_entities();
				for(flecs::entity &e : _new_group->get_entities()) {
					update_selected(e, false);
					dst_entities.erase(std::remove(dst_entities.begin(), dst_entities.end(), e), dst_entities.end());
				}
			}
			else if (_update_type == UpdateType::SET) {
				// deselect old entities
				for(flecs::entity &e : _group->get_entities()) {
					update_selected(e, false);
				}
				_group->get_entities().clear();
				// add new entities
				auto &dst_entities = _group->get_entities();
				for(flecs::entity &e : _new_group->get_entities()) {
					update_selected(e, true);
					insert_if_not_present(dst_entities, e);
				}
			}
			else if (_update_type == UpdateType::CLEAR) {
				// deselect old entities
				for(flecs::entity &e : _group->get_entities()) {
					update_selected(e, false);
				}
				_group->get_entities().clear();
			}
			_update_type = UpdateType::NONE;
		});
}



void SelectionGroup::set_group(Ref<EntityGroup> group) {
	std::lock_guard<std::mutex> lock(_mutex);
	_new_group = group;
	_update_type = UpdateType::SET;
}
void SelectionGroup::append_to_group(Ref<EntityGroup> group) {
	std::lock_guard<std::mutex> lock(_mutex);
	_new_group = group;
	_update_type = UpdateType::ADD;
}
void SelectionGroup::delete_from_group(Ref<EntityGroup> group) {
	std::lock_guard<std::mutex> lock(_mutex);
	_new_group = group;
	_update_type = UpdateType::REMOVE;
}
void SelectionGroup::clear_group() {
	std::lock_guard<std::mutex> lock(_mutex);
		_update_type = UpdateType::CLEAR;
}

Ref<EntityGroup> SelectionGroup::group() const {
	std::lock_guard<std::mutex> lock(_mutex);
	return _group;
}

}
