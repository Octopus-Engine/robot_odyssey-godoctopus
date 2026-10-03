#include "DelaunayTriangulationApiNode.h"

#include "core/object/class_db.h"
#include "godoctopus/game/GameNode.h"

// #define DEBUG_CALLS

namespace godot {

std::vector<DelaunayTriangulationApiNode::RenderTriangle> DelaunayTriangulationApiNode::get_render_triangles() const {
	std::vector<RenderTriangle> triangles;
	if (!_game_node) {
		return triangles;
	}
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		octopus::DelaunayTriangulation const &triangulation = _game_node->get_delaunay_triangulation();
		auto append_triangles = [&triangles, &triangulation](std::vector<octopus::Triangle> const &source, bool hole) {
			for (octopus::Triangle const &tri : source) {
				RenderTriangle render_triangle;
				render_triangle.hole = hole;
				for (size_t i = 0; i < 3; ++i) {
					octopus::TriPoint const &point = triangulation.point(tri.v[i]);
					render_triangle.points[i] = Vector2(octopus::to_fixed(point.x).to_double(), octopus::to_fixed(point.y).to_double());
				}
				triangles.push_back(render_triangle);
			}
		};
		append_triangles(triangulation.triangles(), false);
		append_triangles(triangulation.holeTriangles(), true);
	}
	return triangles;
}

int DelaunayTriangulationApiNode::add_point(Vector2 const &point) {
	ERR_FAIL_NULL_V(_game_node, -1);
	#ifdef DEBUG_CALLS
		std::cout<<"tri.addPoint(" << point.x << ", " << point.y << ");" << std::endl;
	#endif
	octopus::PointIdx idx;
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		idx = _game_node->get_delaunay_triangulation().addPoint(octopus::Fixed(point.x), octopus::Fixed(point.y));
	}
	return static_cast<int>(idx);
}

void DelaunayTriangulationApiNode::remove_point(int idx) {
	ERR_FAIL_NULL(_game_node);
	#ifdef DEBUG_CALLS
		std::cout<<"tri.removePoint(" << idx << ");" << std::endl;
	#endif
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		_game_node->get_delaunay_triangulation().removePoint(static_cast<octopus::PointIdx>(idx));
	}
}

void DelaunayTriangulationApiNode::remove_points(TypedArray<int> const &indices) {
	ERR_FAIL_NULL(_game_node);
	#ifdef DEBUG_CALLS
		std::cout<<"tri.removePoints({";
		for (int i = 0; i < indices.size(); ++i) {
			std::cout<<(int)indices[i];
			if (i < indices.size() - 1) {
				std::cout<<", ";
			}
		}
		std::cout<<"});" << std::endl;
	#endif
	std::vector<octopus::PointIdx> points_to_remove;
	for (int i = 0; i < indices.size(); ++i) {
		points_to_remove.push_back(static_cast<octopus::PointIdx>(indices[i]));
	}
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		_game_node->get_delaunay_triangulation().removePoints(points_to_remove);
	}
}

int DelaunayTriangulationApiNode::get_point_count() const {
	ERR_FAIL_NULL_V(_game_node, 0);
	std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
	return static_cast<int>(_game_node->get_delaunay_triangulation().pointCount());
}

int DelaunayTriangulationApiNode::get_closest_point_idx(double x, double y) const {
	ERR_FAIL_NULL_V(_game_node, -1);
	std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
	octopus::DelaunayTriangulation const &triangulation = _game_node->get_delaunay_triangulation();
	std::size_t count = triangulation.pointCount();
	if (count == 0) {
		return -1;
	}
	long long qx = octopus::Fixed(x).to_int();
	long long qy = octopus::Fixed(y).to_int();
	int best_idx = 0;
	octopus::TriPoint const &first = triangulation.point(0);
	long long dx0 = first.x - qx, dy0 = first.y - qy;
	long long best_dist2 = dx0 * dx0 + dy0 * dy0;
	for (std::size_t i = 1; i < count; ++i) {
		octopus::TriPoint const &p = triangulation.point(i);
		long long dx = p.x - qx, dy = p.y - qy;
		long long dist2 = dx * dx + dy * dy;
		if (dist2 < best_dist2) {
			best_dist2 = dist2;
			best_idx = static_cast<int>(i);
		}
	}
	return best_idx;
}

void DelaunayTriangulationApiNode::add_constrained_edge(int a, int b) {
	ERR_FAIL_NULL(_game_node);
	#ifdef DEBUG_CALLS
		std::cout<<"tri.addConstrainedEdge(" << a << ", " << b << ");" << std::endl;
	#endif
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		_game_node->get_delaunay_triangulation().addConstrainedEdge(static_cast<octopus::PointIdx>(a), static_cast<octopus::PointIdx>(b));
	}
}

void DelaunayTriangulationApiNode::remove_constrained_edge(int a, int b) {
	ERR_FAIL_NULL(_game_node);
	#ifdef DEBUG_CALLS
		std::cout<<"tri.removeConstrainedEdge(" << a << ", " << b << ");" << std::endl;
	#endif
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		_game_node->get_delaunay_triangulation().removeConstrainedEdge(static_cast<octopus::PointIdx>(a), static_cast<octopus::PointIdx>(b));
	}
}

bool DelaunayTriangulationApiNode::is_constrained(int a, int b) const {
	ERR_FAIL_NULL_V(_game_node, false);
	std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
	return _game_node->get_delaunay_triangulation().isConstrained(static_cast<octopus::PointIdx>(a), static_cast<octopus::PointIdx>(b));
}

void DelaunayTriangulationApiNode::mark_hole(Array const &indices) {
	ERR_FAIL_NULL(_game_node);
	std::vector<octopus::PointIdx> polygon;
	polygon.reserve(static_cast<std::size_t>(indices.size()));
	for (int i = 0; i < indices.size(); ++i) {
		polygon.push_back(static_cast<octopus::PointIdx>(static_cast<int>(indices[i])));
	}
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		_game_node->get_delaunay_triangulation().markHole(polygon);
	}
}

void DelaunayTriangulationApiNode::clear_holes() {
	ERR_FAIL_NULL(_game_node);
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		_game_node->get_delaunay_triangulation().clearHoles();
	}
}

TypedArray<Vector2> DelaunayTriangulationApiNode::find_path(Vector2 const &start, Vector2 const &end) const {
	ERR_FAIL_NULL_V(_game_node, TypedArray<Vector2>());
	std::vector<octopus::Vector> path;
	{
		std::lock_guard<std::mutex> lock(_game_node->get_progress_mutex());
		path = _game_node->get_delaunay_triangulation_navigator().compute_funnel(
			octopus::Vector(start.x, start.y),
			octopus::Vector(end.x, end.y)
		);
	}
	TypedArray<Vector2> result;
	result.resize(static_cast<int>(path.size()));
	for (size_t i = 0; i < path.size(); ++i) {
		result[i] = Vector2(path[i].x.to_double(), path[i].y.to_double());
	}
	return result;
}

void DelaunayTriangulationApiNode::_bind_methods() {
	BIND_NODE_PATH(DelaunayTriangulationApiNode, GameNode, game_node);
	ClassDB::bind_method(D_METHOD("add_point", "point"), &DelaunayTriangulationApiNode::add_point);
	ClassDB::bind_method(D_METHOD("remove_point", "idx"), &DelaunayTriangulationApiNode::remove_point);
	ClassDB::bind_method(D_METHOD("remove_points", "indices"), &DelaunayTriangulationApiNode::remove_points);
	ClassDB::bind_method(D_METHOD("get_point_count"), &DelaunayTriangulationApiNode::get_point_count);
	ClassDB::bind_method(D_METHOD("get_closest_point_idx", "x", "y"), &DelaunayTriangulationApiNode::get_closest_point_idx);
	ClassDB::bind_method(D_METHOD("add_constrained_edge", "a", "b"), &DelaunayTriangulationApiNode::add_constrained_edge);
	ClassDB::bind_method(D_METHOD("remove_constrained_edge", "a", "b"), &DelaunayTriangulationApiNode::remove_constrained_edge);
	ClassDB::bind_method(D_METHOD("is_constrained", "a", "b"), &DelaunayTriangulationApiNode::is_constrained);
	ClassDB::bind_method(D_METHOD("mark_hole", "indices"), &DelaunayTriangulationApiNode::mark_hole);
	ClassDB::bind_method(D_METHOD("clear_holes"), &DelaunayTriangulationApiNode::clear_holes);
	ClassDB::bind_method(D_METHOD("find_path", "start", "end"), &DelaunayTriangulationApiNode::find_path);
}

void DelaunayTriangulationApiNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			INIT_NODE_PATH(GameNode, game_node);
			set_process(false);
		} break;
	}
}

} // namespace godot
