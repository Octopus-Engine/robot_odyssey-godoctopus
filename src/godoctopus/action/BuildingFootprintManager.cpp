#include "BuildingFootprintManager.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace godot {

namespace {

int64_t floor_div(int64_t value, int64_t divisor) {
	int64_t quotient = value / divisor;
	if (value % divisor < 0) {
		--quotient;
	}
	return quotient;
}

int64_t abs_int64(int64_t value) {
	return value < 0 ? -value : value;
}

std::pair<octopus::PointIdx, octopus::PointIdx> make_triangulation_edge(
	octopus::PointIdx a,
	octopus::PointIdx b
) {
	return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
}

octopus::PointIdx remap_point_index(
	octopus::PointIdx index,
	std::vector<octopus::PointIdx> const &removed
) {
	auto const end = std::lower_bound(removed.begin(), removed.end(), index);
	return index - static_cast<octopus::PointIdx>(std::distance(removed.begin(), end));
}

}

int64_t BuildingFootprintManager::snap_to_grid(octopus::Fixed value) {
	int64_t const scaled = octopus::to_tri_coord(value);
	return scaled >= 0 ? (scaled + 50) / 100 : (scaled - 50) / 100;
}

std::vector<BuildingFootprintManager::GridCell> BuildingFootprintManager::get_grid_cells(
	octopus::Fixed const &center_x,
	octopus::Fixed const &center_y,
	octopus::Fixed const &radius
) {
	int64_t const center_x_x100 = octopus::to_tri_coord(center_x);
	int64_t const center_y_x100 = octopus::to_tri_coord(center_y);
	int64_t const radius_x100 = abs_int64(octopus::to_tri_coord(radius));
	int64_t const min_x = floor_div(center_x_x100 - radius_x100, 100) - 1;
	int64_t const max_x = floor_div(center_x_x100 + radius_x100, 100) + 1;
	int64_t const min_y = floor_div(center_y_x100 - radius_x100, 100) - 1;
	int64_t const max_y = floor_div(center_y_x100 + radius_x100, 100) + 1;
	std::vector<GridCell> cells;
	int64_t const radius_squared = radius_x100 * radius_x100;

	for (int64_t x = min_x; x <= max_x; ++x) {
		for (int64_t y = min_y; y <= max_y; ++y) {
			int64_t dx = abs_int64(center_x_x100 - x * 100) - 50;
			int64_t dy = abs_int64(center_y_x100 - y * 100) - 50;
			dx = std::max<int64_t>(dx, 0);
			dy = std::max<int64_t>(dy, 0);
			if (dx * dx + dy * dy <= radius_squared) {
				cells.emplace_back(x, y);
			}
		}
	}
	return cells;
}

bool BuildingFootprintManager::overlaps(
	std::vector<GridCell> const &requested_cells,
	std::set<GridCell> const &occupied_cells
) const {
	for (GridCell const &cell : requested_cells) {
		if (occupied_cells.count(cell) > 0) {
			return true;
		}
		for (auto const &[request_id, cells] : _pending_placement_cells) {
			(void)request_id;
			if (std::find(cells.begin(), cells.end(), cell) != cells.end()) {
				return true;
			}
		}
	}
	return false;
}

void BuildingFootprintManager::reserve_cells(int64_t request_id, std::vector<GridCell> const &cells) {
	_pending_placement_cells[request_id] = cells;
}

void BuildingFootprintManager::release_cells(int64_t request_id) {
	_pending_placement_cells.erase(request_id);
}

std::set<BuildingFootprintManager::GridCell> BuildingFootprintManager::get_pending_cells(
	int64_t min_x,
	int64_t min_y,
	int64_t max_x,
	int64_t max_y
) const {
	std::set<GridCell> cells;
	for (auto const &[request_id, reserved_cells] : _pending_placement_cells) {
		(void)request_id;
		for (GridCell const &cell : reserved_cells) {
			if (cell.first >= min_x && cell.first <= max_x && cell.second >= min_y && cell.second <= max_y) {
				cells.insert(cell);
			}
		}
	}
	return cells;
}

void BuildingFootprintManager::add_building(
	flecs::entity entity,
	Vector2 const &position,
	int ray_x100,
	octopus::DelaunayTriangulation &triangulation
) {
	octopus::Fixed const radius = octopus::Fixed(ray_x100) / 100;
	octopus::Fixed const diagonal = radius * octopus::Fixed(707) / 1000;
	constexpr std::array<std::array<int, 2>, 8> directions = {{
		{{1000, 0}},
		{{707, 707}},
		{{0, 1000}},
		{{-707, 707}},
		{{-1000, 0}},
		{{-707, -707}},
		{{0, -1000}},
		{{707, -707}},
	}};

	octopus::Fixed const center_x(position.x);
	octopus::Fixed const center_y(position.y);
	BuildingFootprint footprint;
	footprint.points.reserve(directions.size());
	for (std::array<int, 2> const &direction : directions) {
		octopus::Fixed const offset_x = direction[0] == 1000 ? radius :
			(direction[0] == -1000 ? -radius :
				(direction[0] == 0 ? octopus::Fixed::Zero() : diagonal * (direction[0] > 0 ? 1 : -1)));
		octopus::Fixed const offset_y = direction[1] == 1000 ? radius :
			(direction[1] == -1000 ? -radius :
				(direction[1] == 0 ? octopus::Fixed::Zero() : diagonal * (direction[1] > 0 ? 1 : -1)));
		std::size_t const point_count = triangulation.pointCount();
		octopus::PointIdx const point = triangulation.addPoint(center_x + offset_x, center_y + offset_y);
		footprint.points.push_back(point);
		++_building_point_references[point];
		if (triangulation.pointCount() > point_count) {
			_building_owned_points.insert(point);
		}
	}

	footprint.edges.reserve(footprint.points.size());
	for (std::size_t i = 0; i < footprint.points.size(); ++i) {
		octopus::PointIdx const a = footprint.points[i];
		octopus::PointIdx const b = footprint.points[(i + 1) % footprint.points.size()];
		TriangulationEdge const edge = make_triangulation_edge(a, b);
		footprint.edges.push_back(edge);
		auto const existing = _building_edge_references.find(edge);
		if (existing != _building_edge_references.end()) {
			++existing->second;
			continue;
		}

		bool const was_constrained = triangulation.isConstrained(a, b);
		triangulation.addConstrainedEdge(a, b);
		_building_edge_references.emplace(edge, 1);
		if (!was_constrained) {
			_building_owned_edges.insert(edge);
		}
	}
	triangulation.markHole(footprint.points);
	_building_footprints[entity.id()] = std::move(footprint);
}

void BuildingFootprintManager::remove_building(
	flecs::entity_t entity_id,
	octopus::DelaunayTriangulation &triangulation
) {
	auto const footprint_it = _building_footprints.find(entity_id);
	if (footprint_it == _building_footprints.end()) {
		return;
	}

	BuildingFootprint const footprint = footprint_it->second;
	triangulation.removeHole(footprint.points);

	for (TriangulationEdge const &edge : footprint.edges) {
		auto const reference = _building_edge_references.find(edge);
		if (reference == _building_edge_references.end()) {
			continue;
		}
		if (--reference->second > 0) {
			continue;
		}
		if (_building_owned_edges.erase(edge) > 0) {
			triangulation.removeConstrainedEdge(edge.first, edge.second);
		}
		_building_edge_references.erase(reference);
	}

	std::vector<octopus::PointIdx> points_to_remove;
	for (octopus::PointIdx const point : footprint.points) {
		auto const reference = _building_point_references.find(point);
		if (reference == _building_point_references.end()) {
			continue;
		}
		if (--reference->second > 0) {
			continue;
		}
		_building_point_references.erase(reference);
		bool const owned = _building_owned_points.erase(point) > 0;
		if (owned && !triangulation.isPointConstrained(point)) {
			points_to_remove.push_back(point);
		}
	}

	_building_footprints.erase(footprint_it);
	std::sort(points_to_remove.begin(), points_to_remove.end());
	points_to_remove.erase(std::unique(points_to_remove.begin(), points_to_remove.end()), points_to_remove.end());
	if (!points_to_remove.empty()) {
		triangulation.removePoints(points_to_remove);

		for (auto &[id, remaining] : _building_footprints) {
			(void)id;
			for (octopus::PointIdx &point : remaining.points) {
				point = remap_point_index(point, points_to_remove);
			}
			for (TriangulationEdge &edge : remaining.edges) {
				edge = make_triangulation_edge(
					remap_point_index(edge.first, points_to_remove),
					remap_point_index(edge.second, points_to_remove));
			}
		}

		std::map<octopus::PointIdx, std::size_t> remapped_point_references;
		for (auto const &[point, references] : _building_point_references) {
			remapped_point_references.emplace(remap_point_index(point, points_to_remove), references);
		}
		_building_point_references.swap(remapped_point_references);

		std::set<octopus::PointIdx> remapped_owned_points;
		for (octopus::PointIdx const point : _building_owned_points) {
			remapped_owned_points.insert(remap_point_index(point, points_to_remove));
		}
		_building_owned_points.swap(remapped_owned_points);

		std::map<TriangulationEdge, std::size_t> remapped_edge_references;
		for (auto const &[edge, references] : _building_edge_references) {
			remapped_edge_references.emplace(
				make_triangulation_edge(
					remap_point_index(edge.first, points_to_remove),
					remap_point_index(edge.second, points_to_remove)),
				references);
		}
		_building_edge_references.swap(remapped_edge_references);

		std::set<TriangulationEdge> remapped_owned_edges;
		for (TriangulationEdge const &edge : _building_owned_edges) {
			remapped_owned_edges.insert(make_triangulation_edge(
				remap_point_index(edge.first, points_to_remove),
				remap_point_index(edge.second, points_to_remove)));
		}
		_building_owned_edges.swap(remapped_owned_edges);
	}
}

}
