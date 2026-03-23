#include "godoctopus/triangulation/TriangulationNode.h"

namespace octopus::triangulation {

ssize_t find_triangle_containing_point(Triangulation const &triangulation, Vector const &point) {
	for (size_t i = 0; i < triangulation.triangles.size(); ++i) {
		Triangle const &tri = triangulation.triangles[i];
		Vector const &A = triangulation.vertices[tri.vertices[0]];
		Vector const &B = triangulation.vertices[tri.vertices[1]];
		Vector const &C = triangulation.vertices[tri.vertices[2]];

		// Compute cross products to check if point is inside triangle (assuming CCW order)
		Fixed crossAB = (B.x - A.x) * (point.y - A.y) - (B.y - A.y) * (point.x - A.x);
		Fixed crossBC = (C.x - B.x) * (point.y - B.y) - (C.y - B.y) * (point.x - B.x);
		Fixed crossCA = (A.x - C.x) * (point.y - C.y) - (A.y - C.y) * (point.x - C.x);

		// If all cross products are positive (for CCW triangle), point is inside
		if (crossAB > 0 && crossBC > 0 && crossCA > 0) {
			return i;
		}
	}
	return -1;
}

ssize_t find_closest_point(Triangulation const &triangulation, Vector const &point) {
	ssize_t closest_vertex_index = -1;
	Fixed closest_distance_squared = 1000000; // some large number
	for (size_t i = 0; i < triangulation.vertices.size(); ++i) {
		Vector const &v = triangulation.vertices[i];
		Fixed dx = v.x - point.x;
		Fixed dy = v.y - point.y;
		Fixed distance_squared = dx*dx + dy*dy;
		if (distance_squared < closest_distance_squared) {
			closest_distance_squared = distance_squared;
			closest_vertex_index = i;
		}
	}
	return closest_vertex_index;
}

void insert_point(Triangulation &triangulation, Vector const &point) {
	ssize_t tri_idx = find_triangle_containing_point(triangulation, point);
	if (tri_idx == -1) {
		// Point is outside all triangles, do not insert (for now)
		return;
	}
	Triangle &tri = triangulation.triangles[tri_idx];
	size_t v0 = tri.vertices[0];
	size_t v1 = tri.vertices[1];
	size_t v2 = tri.vertices[2];

	size_t o0 = tri.opposites[0];
	size_t o1 = tri.opposites[1];
	size_t o2 = tri.opposites[2];

	size_t vP = triangulation.add_vertex(point);
	// Replace the original triangle with three new triangles
	triangulation.triangles[tri_idx] = {{v0, v1, vP}};
	triangulation.triangles.push_back({{v1, v2, vP}});
	triangulation.triangles.push_back({{v2, v0, vP}});

	// name triangles based on the original triangle vertex NOT in the triangle
	size_t t2 = tri_idx;
	size_t t0 = triangulation.triangles.size() - 2;
	size_t t1 = triangulation.triangles.size() - 1;

	// connect original opposites for the new triangles
	connect_opposites(triangulation, t0, o0);
	connect_opposites(triangulation, t1, o1);
	connect_opposites(triangulation, t2, o2);

	connect_opposites(triangulation, t0, t1);
	connect_opposites(triangulation, t1, t2);
	connect_opposites(triangulation, t2, t0);

	// make sure all triangles are CCW
	triangulation.make_triangle_ccw(t0);
	triangulation.make_triangle_ccw(t1);
	triangulation.make_triangle_ccw(t2);

	// Update opposite triangle references
}

}
