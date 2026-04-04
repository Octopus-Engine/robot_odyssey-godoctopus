#pragma once

#include "scene/2d/node_2d.h"

#include "octopus/utils/FixedPoint.hh"
#include "octopus/utils/Vector.hh"

#include <vector>
#include <array>
#include <algorithm>

namespace octopus::triangulation {

struct Triangle {
	std::array<size_t, 3> vertices;
	std::array<size_t, 3> opposites {99999, 99999, 99999}; // indices of opposite triangles, 99999 if no opposite
};

size_t find_opposite_vertex_idx(Triangle const &tri1, Triangle const &tri2);

struct Triangulation;

void connect_opposites(Triangulation &triangulation, size_t tri1, size_t tri2);

struct Triangulation {
	std::vector<Vector> vertices;
	std::vector<Triangle> triangles;

	size_t add_vertex(Vector const &v) {
		for (size_t i = 0; i < vertices.size(); ++i) {
			if (vertices[i] == v) {
				return i;
			}
		}
		vertices.push_back(v);
		return vertices.size() - 1;
	}

	size_t add_triangle(Vector const &v1, Vector const &v2, Vector const &v3) {
		size_t i1 = add_vertex(v1);
		size_t i2 = add_vertex(v2);
		size_t i3 = add_vertex(v3);

		triangles.push_back({{i1, i2, i3}});
		make_triangle_ccw(triangles.size() - 1);
		auto &tri = triangles.back();

		// find opposite triangles and set them
		for (size_t edge = 0; edge < 3; ++edge) {
			size_t vA = tri.vertices[(edge + 1) % 3];
			size_t vB = tri.vertices[(edge + 2) % 3];
			auto opposite_triangles = find_triangle_using_edge(vA, vB);
			if (opposite_triangles.size() == 2) {
				connect_opposites(*this, opposite_triangles[0], opposite_triangles[1]);
			}
		}

		return triangles.size() - 1;
	}

	Vector const &get_vertex(size_t index) const {
		return vertices[index];
	}

	Triangle const &get_triangle(size_t index) const {
		return triangles[index];
	}

	void make_triangle_ccw(size_t triangle_index) {
		Triangle &tri = triangles[triangle_index];
		Vector const &A = vertices[tri.vertices[0]];
		Vector const &B = vertices[tri.vertices[1]];
		Vector const &C = vertices[tri.vertices[2]];
		Fixed cross = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
		if (cross < 0) {
			std::swap(tri.vertices[1], tri.vertices[2]);
			std::swap(tri.opposites[1], tri.opposites[2]);
		}
	}

	std::vector<size_t> find_triangles_using_vertex(size_t vertex_index) const {
		std::vector<size_t> result;
		for (size_t i = 0; i < triangles.size(); ++i) {
			const Triangle &tri = triangles[i];
			if (tri.vertices[0] == vertex_index || tri.vertices[1] == vertex_index || tri.vertices[2] == vertex_index) {
				result.push_back(i);
			}
		}
		return result;
	}

	std::vector<size_t> find_triangle_using_edge(size_t vertex_index1, size_t vertex_index2) const {
		std::vector<size_t> result;
		for (size_t i = 0; i < triangles.size(); ++i) {
			const Triangle &tri = triangles[i];
			bool has_v1 = (tri.vertices[0] == vertex_index1 || tri.vertices[1] == vertex_index1 || tri.vertices[2] == vertex_index1);
			bool has_v2 = (tri.vertices[0] == vertex_index2 || tri.vertices[1] == vertex_index2 || tri.vertices[2] == vertex_index2);
			if (has_v1 && has_v2) {
				result.push_back(i);
			}
		}
		return result;
	}
};

void connect_opposites(Triangulation &triangulation, size_t tri1, size_t tri2);

size_t find_triangle_containing_point(Triangulation const &triangulation, Vector const &point);
size_t find_closest_point(Triangulation const &triangulation, Vector const &point);
void insert_point(Triangulation &triangulation, Vector const &point);

}

namespace godot {

class TriangulationNode : public Node2D {
	GDCLASS(TriangulationNode, Node2D)

	void _process(double delta);

	void _draw();

	void add_triangle(Vector2 const &v1, Vector2 const &v2, Vector2 const &v3);

	void add_point(Vector2 const &point);
	void select_triangle(Vector2 const &point);
	void select_vertex(Vector2 const &point);
	void unselect_vertex();

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();
protected:
	void _notification(int p_notification);

	void draw_triangle(size_t triangle_index, Color color);

	octopus::triangulation::Triangulation triangulation;
	size_t selected_triangle_index = -1;
	size_t selected_vertex_index = -1;
};

}
