#include "testing/DelaunayPathFindingCache.test.h"

#include "tests/test_macros.h"

#include "flecs.h"
#include "octopus/components/basic/position/Position.hh"
#include "octopus/systems/Systems.hh"
#include "octopus/triangulation/DelaunayPathFindingCache.hh"
#include "octopus/triangulation/DelaunayTriangulationNavigator.hh"
#include "octopus/world/stats/TimeStats.hh"

void test_delaunay_path_cache_invalidates_after_mesh_change() {
	octopus::DelaunayTriangulation mesh;
	std::vector<octopus::PointIdx> const boundary = {
		mesh.addPoint(octopus::Fixed(-100), octopus::Fixed(-100)),
		mesh.addPoint(octopus::Fixed(600), octopus::Fixed(-100)),
		mesh.addPoint(octopus::Fixed(600), octopus::Fixed(600)),
		mesh.addPoint(octopus::Fixed(-100), octopus::Fixed(600))
	};
	for (std::size_t i = 0; i < boundary.size(); ++i) {
		mesh.addConstrainedEdge(boundary[i], boundary[(i + 1) % boundary.size()]);
	}

	octopus::DelaunayTriangulationNavigator navigator(mesh);
	octopus::DelaunayPathFindingCache cache(navigator);
	TimeStats stats;
	flecs::world ecs;
	octopus::set_up_phases(ecs);
	cache.declare_cache_update_system(ecs, stats);

	octopus::Position const position {{10, 30}};
	octopus::DelaunayPathQuery const stale_query = cache.query_path(position, {50, 30});
	CHECK(!stale_query.is_valid());
	ecs.progress();
	CHECK(stale_query.is_valid());
	CHECK(stale_query.get_direction() == octopus::Vector(40, 0));

	octopus::Position const pending_position {{10, -30}};
	octopus::DelaunayPathQuery const pending_query =
		cache.query_path(pending_position, {50, -30});
	CHECK(!pending_query.is_valid());

	std::vector<octopus::PointIdx> const obstacle = {
		mesh.addPoint(octopus::Fixed(20), octopus::Fixed(20)),
		mesh.addPoint(octopus::Fixed(40), octopus::Fixed(20)),
		mesh.addPoint(octopus::Fixed(40), octopus::Fixed(40)),
		mesh.addPoint(octopus::Fixed(20), octopus::Fixed(40))
	};
	mesh.markHole(obstacle);

	CHECK(!stale_query.is_valid());
	CHECK(stale_query.get_direction() == octopus::Vector());
	CHECK(!pending_query.is_valid());
	CHECK(pending_query.get_direction() == octopus::Vector());

	octopus::DelaunayPathQuery const updated_query = cache.query_path(position, {50, 30});
	CHECK(!updated_query.is_valid());
	CHECK(updated_query.result != stale_query.result);
	ecs.progress();
	CHECK(updated_query.is_valid());
	CHECK(updated_query.get_direction() == octopus::Vector(10, -10));
	CHECK(!pending_query.is_valid());
}
