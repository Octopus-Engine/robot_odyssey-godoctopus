#include "HealthBarNode.h"

#include "octopus/components/basic/position/Position.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

#include "flecs.h"

namespace godot {

HealthBarNode::~HealthBarNode() {
	bars.for_each_const([&](HealthBarData const &bar) {
		RenderingServer::get_singleton()->free(bar.rid);
	});
}

int HealthBarNode::add_health_bar() {
	std::lock_guard<std::mutex> lock(mutex);
	return bars.new_instance(HealthBarData{}).handle();
}

void HealthBarNode::free_health_bar(int idx) {
	std::lock_guard<std::mutex> lock(mutex);
	RenderingServer::get_singleton()->free(bars.get(idx).rid);
	bars.free_instance(idx);
}

void HealthBarNode::set_bar_position(int idx, Vector3 pos) {
	std::lock_guard<std::mutex> lock(mutex);
	bars.get(idx).pos = pos;
}

void HealthBarNode::set_bar_ratio(int idx, float ratio) {
	std::lock_guard<std::mutex> lock(mutex);
	bars.get(idx).ratio = ratio;
}

void HealthBarNode::_process(double delta) {
	if (!_camera || !_health_bar_control_container) {return;}
	std::lock_guard<std::mutex> lock(mutex);
	double ratio = 25. / _camera->get_size();
	Vector2 s = Vector2(ratio, std::min(2., std::max(1.,ratio)));
	Vector2 csize = Vector2(1.*s.x,3*s.y);

	bars.for_each([&](HealthBarData &bar) {
		if (bar.rid.is_null()) {
			RID rid = RenderingServer::get_singleton()->canvas_item_create();
			RenderingServer::get_singleton()->canvas_item_set_material(rid, bar_material->get_rid());
			RenderingServer::get_singleton()->canvas_item_set_parent(rid, _health_bar_control_container->get_canvas_item());
			bar.rid = rid;
		} else {
			RenderingServer::get_singleton()->canvas_item_clear(bar.rid);
		}
		// Never
		if (display_mode == 2) {
			return;
		}
		// Damaged only
		if (display_mode == 1 && bar.ratio > 0.99) {
			return;
		}
		RenderingServer::get_singleton()->canvas_item_set_transform(bar.rid, Transform2D().translated(
			_camera->unproject_position(bar.pos) + Vector2(-csize.x*bar.width/2, 0)
		));
		RenderingServer::get_singleton()->canvas_item_add_texture_rect(bar.rid, Rect2(0,0,csize.x*bar.width,csize.y), texture->get_rid(), true);
		RenderingServer::get_singleton()->canvas_item_set_modulate(bar.rid, Color(bar.ratio,1,1,1));
	});
	// need to redraw
	_health_bar_control_container->queue_redraw();
}

void HealthBarNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
			if (_game_node && _health_bar_control_container && _camera) {
				_game_node->connect("init_done", callable_mp(this, &HealthBarNode::setup));
			}
			set_process(true);
		} break;
	}
}

void HealthBarNode::setup() {
	flecs::world &ecs = _game_node->get_world().ecs;
	ecs.component<HealthBar>()
		.member("offset", &HealthBar::offset)
		.member("width", &HealthBar::width)
	;

	ecs.system<octopus::Position const, octopus::HitPoint const, octopus::HitPointMax const, HealthBar>()
		.kind(ecs.entity(DisplaySyncPhase))
		.each([this](flecs::entity e, octopus::Position const &pos, octopus::HitPoint const &hp, octopus::HitPointMax const &hp_max, HealthBar &hp_bar) {
			if(hp_bar.idx_bar < 0) {
				hp_bar.idx_bar = this->add_health_bar();
			}
			std	::lock_guard<std::mutex> lock(this->mutex);
			// sync
			HealthBarData &hp_data = bars[hp_bar.idx_bar];
			hp_data.pos = WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), hp_bar.offset, real_t(octopus::to_double(pos.pos.y)));
			hp_data.width = hp_bar.width;
			hp_data.ratio = hp.qty.to_double()/hp_max.qty.to_double();
		});


	ecs.system<HealthBar>()
		.with(flecs::Disabled)
		.kind(ecs.entity(DisplaySyncPhase))
		.each([this](flecs::entity e, HealthBar &hp_bar) {
			if (hp_bar.idx_bar < 0) {
				return;
			}
			this->free_health_bar(hp_bar.idx_bar);
			e.remove<HealthBar>();
	});
}

}
