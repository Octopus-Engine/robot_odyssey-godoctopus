#include "TriangulationNode.h"

namespace octopus::triangulation {

ssize_t find_opposite_vertex_idx(Triangle const &tri1, Triangle const &tri2) {
	for (size_t edge1 = 0; edge1 < 3; ++edge1) {
	bool found = false;
		for (size_t edge2 = 0; edge2 < 3; ++edge2) {
			if (tri1.vertices[edge1] == tri2.vertices[edge2]) {
				found = true;
				break;
			}
		}
		if (!found) {
			return edge1;
		}
	}
	return -1;
}

void connect_opposites(Triangulation &triangulation, size_t tri1, size_t tri2) {
	ssize_t edge1 = find_opposite_vertex_idx(triangulation.triangles[tri1], triangulation.triangles[tri2]);
	if (edge1 != -1) {
		triangulation.triangles[tri1].opposites[edge1] = tri2;
	}
	ssize_t edge2 = find_opposite_vertex_idx(triangulation.triangles[tri2], triangulation.triangles[tri1]);
	if (edge2 != -1) {
		triangulation.triangles[tri2].opposites[edge2] = tri1;
	}
}

}

namespace godot {

void TriangulationNode::_process(double delta) {
	// TODO
}

void TriangulationNode::draw_triangle(size_t triangle_index, Color color) {
	octopus::triangulation::Triangle const &triangle = triangulation.triangles[triangle_index];
	octopus::Vector const &v1 = triangulation.vertices[triangle.vertices[0]];
	octopus::Vector const &v2 = triangulation.vertices[triangle.vertices[1]];
	octopus::Vector const &v3 = triangulation.vertices[triangle.vertices[2]];
	draw_line(Vector2(v1.x.to_double(), v1.y.to_double()), Vector2(v2.x.to_double(), v2.y.to_double()), color);
	draw_line(Vector2(v2.x.to_double(), v2.y.to_double()), Vector2(v3.x.to_double(), v3.y.to_double()), color);
	draw_line(Vector2(v3.x.to_double(), v3.y.to_double()), Vector2(v1.x.to_double(), v1.y.to_double()), color);
}

void TriangulationNode::_draw() {
	for (size_t i = 0; i < triangulation.triangles.size(); ++i) {
		draw_triangle(i, Color(0, 1, 0));
	}
	if (selected_triangle_index >= 0) {
		draw_triangle(selected_triangle_index, Color(1, 0, 0));
	}

	if (selected_vertex_index >= 0) {
		octopus::Vector const &v = triangulation.vertices[selected_vertex_index];
		draw_circle(Vector2(v.x.to_double(), v.y.to_double()), 5, Color(1, 1, 0));

		if (selected_triangle_index >= 0) {
			for (size_t edge = 0; edge < 3; ++edge) {
				if (triangulation.triangles[selected_triangle_index].vertices[edge] == selected_vertex_index &&
					triangulation.triangles[selected_triangle_index].opposites[edge] != -1) {
					draw_triangle(triangulation.triangles[selected_triangle_index].opposites[edge], Color(0, 0, 1));
				}
			}
		}
	}
}

void TriangulationNode::add_triangle(Vector2 const &v1, Vector2 const &v2, Vector2 const &v3) {
	triangulation.add_triangle(octopus::Vector(v1.x, v1.y), octopus::Vector(v2.x, v2.y), octopus::Vector(v3.x, v3.y));
	queue_redraw();
}

void TriangulationNode::add_point(Vector2 const &point) {
	octopus::triangulation::insert_point(triangulation, octopus::Vector(point.x, point.y));
	queue_redraw();
}

void TriangulationNode::select_triangle(Vector2 const &point) {
	selected_triangle_index = octopus::triangulation::find_triangle_containing_point(triangulation, octopus::Vector(point.x, point.y));
	queue_redraw();
}

void TriangulationNode::select_vertex(Vector2 const &point) {
	selected_vertex_index = octopus::triangulation::find_closest_point(triangulation, octopus::Vector(point.x, point.y));
	queue_redraw();
}

void TriangulationNode::unselect_vertex() {
	selected_vertex_index = -1;
	queue_redraw();
}

void TriangulationNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_triangle", "v1", "v2", "v3"), &TriangulationNode::add_triangle);
	ClassDB::bind_method(D_METHOD("add_point", "point"), &TriangulationNode::add_point);
	ClassDB::bind_method(D_METHOD("select_triangle", "point"), &TriangulationNode::select_triangle);
	ClassDB::bind_method(D_METHOD("select_vertex", "point"), &TriangulationNode::select_vertex);
	ClassDB::bind_method(D_METHOD("unselect_vertex"), &TriangulationNode::unselect_vertex);
}

void TriangulationNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			set_process(true);
		} break;
		case NOTIFICATION_DRAW: {
			_draw();
		} break;
	}
}

}
