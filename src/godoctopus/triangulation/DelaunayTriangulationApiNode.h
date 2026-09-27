#pragma once

#include "scene/main/node.h"

#include "octopus/triangulation/DelaunayTriangulation.hh"
#include "godot_tools.h"

#include <vector>

namespace godot {

class GameNode;

class DelaunayTriangulationApiNode : public Node {
	GDCLASS(DelaunayTriangulationApiNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);

public:
	struct RenderTriangle {
		Vector2 points[3];
		bool hole;
	};

	std::vector<RenderTriangle> get_render_triangles() const;

private:
	// ── Point management ────────────────────────────────────────────────────
	int add_point(Vector2 const &point);
	void remove_point(int idx);
	void remove_points(TypedArray<int> const &indices);
	int get_point_count() const;
	int get_closest_point_idx(double x, double y) const;

	// ── Constrained edges ───────────────────────────────────────────────────
	void add_constrained_edge(int a, int b);
	void remove_constrained_edge(int a, int b);
	bool is_constrained(int a, int b) const;

	// ── Holes ────────────────────────────────────────────────────────────────
	void mark_hole(Array const &indices);
	void clear_holes();

	// ── Navigation ───────────────────────────────────────────────────────────
	TypedArray<Vector2> find_path(Vector2 const &start, Vector2 const &end) const;

	static void _bind_methods();

protected:
	void _notification(int p_notification);
};

} // namespace godot
