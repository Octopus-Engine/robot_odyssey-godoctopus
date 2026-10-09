#pragma once

#include <cstdint>
#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "core/math/vector2.h"
#include "flecs.h"
#include "octopus/triangulation/DelaunayTriangulation.hh"

namespace godot {

class BuildingFootprintManager {
public:
	using GridCell = std::pair<int64_t, int64_t>;

	static int64_t snap_to_grid(octopus::Fixed value);
	static std::vector<GridCell> get_grid_cells(
		octopus::Fixed const &center_x,
		octopus::Fixed const &center_y,
		octopus::Fixed const &radius
	);

	bool overlaps(
		std::vector<GridCell> const &requested_cells,
		std::set<GridCell> const &occupied_cells
	) const;
	void reserve_cells(int64_t request_id, std::vector<GridCell> const &cells);
	void release_cells(int64_t request_id);
	std::set<GridCell> get_pending_cells(
		int64_t min_x,
		int64_t min_y,
		int64_t max_x,
		int64_t max_y
	) const;

	void add_building(flecs::entity entity, Vector2 const &position, int ray_x100, octopus::DelaunayTriangulation &triangulation);
	void remove_building(flecs::entity_t entity_id, octopus::DelaunayTriangulation &triangulation);

private:
	using TriangulationEdge = std::pair<octopus::PointIdx, octopus::PointIdx>;

	struct BuildingFootprint {
		std::vector<octopus::PointIdx> points;
		std::vector<TriangulationEdge> edges;
	};

	std::map<flecs::entity_t, BuildingFootprint> _building_footprints;
	std::map<octopus::PointIdx, std::size_t> _building_point_references;
	std::set<octopus::PointIdx> _building_owned_points;
	std::map<TriangulationEdge, std::size_t> _building_edge_references;
	std::set<TriangulationEdge> _building_owned_edges;
	std::map<int64_t, std::vector<GridCell>> _pending_placement_cells;
};

}
