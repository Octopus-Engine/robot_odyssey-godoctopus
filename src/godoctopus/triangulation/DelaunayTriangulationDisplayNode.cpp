#include "DelaunayTriangulationDisplayNode.h"

#include "core/object/class_db.h"
#include "DelaunayTriangulationApiNode.h"

namespace godot {

void DelaunayTriangulationDisplayNode::_draw() {
	if (!_triangulation_api) {
		return;
	}

	static const Color FILL_NORMAL(0.0f, 1.0f, 0.0f, 0.35f);
	static const Color OUTLINE_NORMAL(0.0f, 1.0f, 0.0f, 1.0f);
	static const Color FILL_HOLE(1.0f, 0.0f, 0.0f, 0.35f);
	static const Color OUTLINE_HOLE(1.0f, 0.0f, 0.0f, 1.0f);

	for (DelaunayTriangulationApiNode::RenderTriangle const &tri : _triangulation_api->get_render_triangles()) {
		Color const &fill = tri.hole ? FILL_HOLE : FILL_NORMAL;
		Color const &outline = tri.hole ? OUTLINE_HOLE : OUTLINE_NORMAL;
		PackedVector2Array points;
		points.push_back(tri.points[0]);
		points.push_back(tri.points[1]);
		points.push_back(tri.points[2]);
		draw_colored_polygon(points, fill);
		draw_line(tri.points[0], tri.points[1], outline);
		draw_line(tri.points[1], tri.points[2], outline);
		draw_line(tri.points[2], tri.points[0], outline);
	}
}

void DelaunayTriangulationDisplayNode::_bind_methods() {
	BIND_NODE_PATH(DelaunayTriangulationDisplayNode, DelaunayTriangulationApiNode, triangulation_api);
}

void DelaunayTriangulationDisplayNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			INIT_NODE_PATH(DelaunayTriangulationApiNode, triangulation_api);
			set_process(_triangulation_api != nullptr);
		} break;
		case NOTIFICATION_PROCESS: {
			queue_redraw();
		} break;
		case NOTIFICATION_DRAW: {
			_draw();
		} break;
	}
}

} // namespace godot
