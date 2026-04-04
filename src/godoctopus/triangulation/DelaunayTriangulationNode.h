#pragma once

#include "scene/2d/node_2d.h"

#include "octopus/triangulation/DelaunayTriangulation.hh"

namespace godot {

class DelaunayTriangulationNode : public Node2D {
	GDCLASS(DelaunayTriangulationNode, Node2D)

	void _draw();

	// ── Point management ────────────────────────────────────────────────────
	int add_point(Vector2 const &point);
	void remove_point(int idx);
	int get_point_count() const;
	int get_closest_point_idx(double x, double y) const;

	// ── Constrained edges ───────────────────────────────────────────────────
	void add_constrained_edge(int a, int b);
	void remove_constrained_edge(int a, int b);
	bool is_constrained(int a, int b) const;

	// ── Holes ────────────────────────────────────────────────────────────────
	void mark_hole(Array const &indices);
	void clear_holes();

	static void _bind_methods();

protected:
	void _notification(int p_notification);

private:
	void draw_triangle(octopus::Triangle const &tri, Color fill_color, Color outline_color);

	octopus::DelaunayTriangulation _triangulation;
};

} // namespace godot
