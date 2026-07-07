#include "DelaunayTriangulationNode.h"

#include "core/object/class_db.h"

namespace godot {

void DelaunayTriangulationNode::draw_triangle(octopus::Triangle const &tri, Color fill_color, Color outline_color) {
	octopus::TriPoint const &p0 = _triangulation.point(tri.v[0]);
	octopus::TriPoint const &p1 = _triangulation.point(tri.v[1]);
	octopus::TriPoint const &p2 = _triangulation.point(tri.v[2]);
	Vector2 v0(p0.x, p0.y);
	Vector2 v1(p1.x, p1.y);
	Vector2 v2(p2.x, p2.y);

	PackedVector2Array pts;
	pts.push_back(v0);
	pts.push_back(v1);
	pts.push_back(v2);
	draw_colored_polygon(pts, fill_color);

	draw_line(v0, v1, outline_color);
	draw_line(v1, v2, outline_color);
	draw_line(v2, v0, outline_color);
}

void DelaunayTriangulationNode::_draw() {
	static const Color FILL_NORMAL(0.0f, 1.0f, 0.0f, 0.35f);
	static const Color OUTLINE_NORMAL(0.0f, 1.0f, 0.0f, 1.0f);
	static const Color FILL_HOLE(1.0f, 0.0f, 0.0f, 0.35f);
	static const Color OUTLINE_HOLE(1.0f, 0.0f, 0.0f, 1.0f);

	for (octopus::Triangle const &tri : _triangulation.triangles()) {
		draw_triangle(tri, FILL_NORMAL, OUTLINE_NORMAL);
	}
	for (octopus::Triangle const &tri : _triangulation.holeTriangles()) {
		draw_triangle(tri, FILL_HOLE, OUTLINE_HOLE);
	}
}

int DelaunayTriangulationNode::add_point(Vector2 const &point) {
	octopus::PointIdx idx = _triangulation.addPoint(octopus::Fixed(point.x), octopus::Fixed(point.y));
	queue_redraw();
	return static_cast<int>(idx);
}

void DelaunayTriangulationNode::remove_point(int idx) {
	_triangulation.removePoint(static_cast<octopus::PointIdx>(idx));
	queue_redraw();
}

int DelaunayTriangulationNode::get_point_count() const {
	return static_cast<int>(_triangulation.pointCount());
}

int DelaunayTriangulationNode::get_closest_point_idx(double x, double y) const {
	std::size_t count = _triangulation.pointCount();
	if (count == 0) {
		return -1;
	}
	long long qx = octopus::Fixed(x).to_int();
	long long qy = octopus::Fixed(y).to_int();
	int best_idx = 0;
	octopus::TriPoint const &first = _triangulation.point(0);
	long long dx0 = first.x - qx, dy0 = first.y - qy;
	long long best_dist2 = dx0 * dx0 + dy0 * dy0;
	for (std::size_t i = 1; i < count; ++i) {
		octopus::TriPoint const &p = _triangulation.point(i);
		long long dx = p.x - qx, dy = p.y - qy;
		long long dist2 = dx * dx + dy * dy;
		if (dist2 < best_dist2) {
			best_dist2 = dist2;
			best_idx = static_cast<int>(i);
		}
	}
	return best_idx;
}

void DelaunayTriangulationNode::add_constrained_edge(int a, int b) {
	_triangulation.addConstrainedEdge(static_cast<octopus::PointIdx>(a), static_cast<octopus::PointIdx>(b));
	queue_redraw();
}

void DelaunayTriangulationNode::remove_constrained_edge(int a, int b) {
	_triangulation.removeConstrainedEdge(static_cast<octopus::PointIdx>(a), static_cast<octopus::PointIdx>(b));
	queue_redraw();
}

bool DelaunayTriangulationNode::is_constrained(int a, int b) const {
	return _triangulation.isConstrained(static_cast<octopus::PointIdx>(a), static_cast<octopus::PointIdx>(b));
}

void DelaunayTriangulationNode::mark_hole(Array const &indices) {
	std::vector<octopus::PointIdx> polygon;
	polygon.reserve(static_cast<std::size_t>(indices.size()));
	for (int i = 0; i < indices.size(); ++i) {
		polygon.push_back(static_cast<octopus::PointIdx>(static_cast<int>(indices[i])));
	}
	_triangulation.markHole(polygon);
	queue_redraw();
}

void DelaunayTriangulationNode::clear_holes() {
	_triangulation.clearHoles();
	queue_redraw();
}

void DelaunayTriangulationNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_point", "point"), &DelaunayTriangulationNode::add_point);
	ClassDB::bind_method(D_METHOD("remove_point", "idx"), &DelaunayTriangulationNode::remove_point);
	ClassDB::bind_method(D_METHOD("get_point_count"), &DelaunayTriangulationNode::get_point_count);
	ClassDB::bind_method(D_METHOD("get_closest_point_idx", "x", "y"), &DelaunayTriangulationNode::get_closest_point_idx);
	ClassDB::bind_method(D_METHOD("add_constrained_edge", "a", "b"), &DelaunayTriangulationNode::add_constrained_edge);
	ClassDB::bind_method(D_METHOD("remove_constrained_edge", "a", "b"), &DelaunayTriangulationNode::remove_constrained_edge);
	ClassDB::bind_method(D_METHOD("is_constrained", "a", "b"), &DelaunayTriangulationNode::is_constrained);
	ClassDB::bind_method(D_METHOD("mark_hole", "indices"), &DelaunayTriangulationNode::mark_hole);
	ClassDB::bind_method(D_METHOD("clear_holes"), &DelaunayTriangulationNode::clear_holes);
}

void DelaunayTriangulationNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_DRAW: {
			_draw();
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
		} break;
	}
}

} // namespace godot
