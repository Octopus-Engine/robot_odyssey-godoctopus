#include "HealthBarNode.h"

namespace godot {

int HealthBarNode::add_health_bar() {
	std::lock_guard<std::mutex> lock(mutex);
	RID rid = RenderingServer::get_singleton()->canvas_item_create();
	RenderingServer::get_singleton()->canvas_item_set_material(rid, bar_material->get_rid());
	RenderingServer::get_singleton()->canvas_item_add_texture_rect(rid, Rect2(0,0,48,48), texture->get_rid(), true);
	RenderingServer::get_singleton()->canvas_item_set_parent(rid, get_canvas_item());
	return bars.new_instance({rid}).handle();
}

void HealthBarNode::free_health_bar(int idx) {
	std::lock_guard<std::mutex> lock(mutex);
	RenderingServer::get_singleton()->free(bars.get(idx).rid);
	bars.free_instance(idx);
}

void HealthBarNode::set_bar_position(int idx, Vector3 pos) {
	std::lock_guard<std::mutex> lock(mutex);
	bars.get(idx).pos = pos;
	// control.position = cam.unproject_position(parent.global_position + Vector3(0,2,0)) + Vector2(-control.size.x/2, 0)
}

void HealthBarNode::set_bar_ratio(int idx, float ratio) {
	std::lock_guard<std::mutex> lock(mutex);
	bars.get(idx).ratio = ratio;
}

void HealthBarNode::_process(double delta) {
	if (!_camera) {return;}
	std::lock_guard<std::mutex> lock(mutex);
	double ratio = 25. / _camera->get_size();
	Vector2 s = Vector2(ratio, std::min(2., std::max(1.,ratio)));
	Vector2 csize = Vector2(32.*s.x,3*s.y);

	bars.for_each_const([&](HealthBarData const &bar) {
		RenderingServer::get_singleton()->canvas_item_clear(bar.rid);
		RenderingServer::get_singleton()->canvas_item_set_transform(bar.rid, Transform2D().translated(
			_camera->unproject_position(bar.pos) + Vector2(-csize.x/2, 0)
		));
		RenderingServer::get_singleton()->canvas_item_add_texture_rect(bar.rid, Rect2(0,0,csize.x,csize.y), texture->get_rid(), true);
		RenderingServer::get_singleton()->canvas_item_set_instance_shader_parameter(bar.rid, "ratio", bar.ratio);
	});
	// need to redraw
	queue_redraw();
}

void HealthBarNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
			set_process(true);
		} break;
	}
}


}
