
#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/components/building/Building.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "testing/ActionNodeBasic.test.h"
#include "octopus/components/basic/position/Position.hh"

struct ActionNodeTestContext {
	godot::GameNode *game_node = nullptr;
	godot::ActionNode *action_node = nullptr;
	godot::InfoProxyNode *proxy_node = nullptr;

	ActionNodeTestContext() {

		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");
		auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		prefab->set_prefab_name("gunbot");
		game_node->get_unit_prefabs().push_back(prefab);

		auto building_prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		building_prefab->set_prefab_name("tallbot");
		building_prefab->set_is_building(true);
		building_prefab->set_is_static(true);
		building_prefab->set_ray_x100(100);
		game_node->get_unit_prefabs().push_back(building_prefab);

		action_node = memnew(godot::ActionNode);
		action_node->set_ref_game_node(NodePath("/root/GameNode"));
		game_node->add_child(action_node);

		proxy_node = memnew(godot::InfoProxyNode);
		proxy_node->set_ref_game_node(NodePath("/root/GameNode"));
		proxy_node->set_refresh_tick(1);
		game_node->add_child(proxy_node);

		SceneTree::get_singleton()->get_root()->add_child(game_node);

		game_node->init_from_level(Dictionary());
	}

	~ActionNodeTestContext() {}
};

void test_actionnode_kill_dummy() {
	ActionNodeTestContext context;

	StringName unit_name = "gunbot";
	Ref<godot::EntityGroup> group(memnew(godot::EntityGroup));
	context.action_node->spawn_dummy_units_in_group(unit_name, Vector2(10, 20), 1, 1, group);
	context.game_node->tick();
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 1);
	}
	// Kill the dummy unit
	context.action_node->kill_units(group);
	context.game_node->tick();

	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 0);
	}
}

void test_actionnode_kill_non_dummy() {
	ActionNodeTestContext context;

	StringName unit_name = "gunbot";
	Ref<godot::EntityGroup> group(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(10, 20), 1, 1, group);
	context.game_node->tick();
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 1);
	}
	// Kill the non-dummy unit
	context.action_node->kill_units(group);
	// Need two ticks to make unit removal effective
	context.game_node->tick(2);

	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 0);
	}
}

void test_actionnode_place_building_grid_and_remove_hole() {
	godot::BuildingFootprintManager footprint_manager;
	std::vector<godot::BuildingFootprintManager::GridCell> pending = {
		{-2, -2},
		{0, 0},
		{2, 2},
	};
	footprint_manager.reserve_cells(1, pending);
	std::set<godot::BuildingFootprintManager::GridCell> pending_snapshot =
		footprint_manager.get_pending_cells(-1, -1, 1, 1);
	CHECK(pending_snapshot.size() == 1);
	CHECK(pending_snapshot.count({0, 0}) == 1);
	footprint_manager.release_cells(1);
	CHECK(footprint_manager.get_pending_cells(-1, -1, 1, 1).empty());

	ActionNodeTestContext context;

	Dictionary initial_preview = context.action_node->get_building_placement_preview("tallbot", Vector2(0.4, 0.4));
	CHECK(initial_preview["valid"].booleanize());
	CHECK(Vector2(initial_preview["position"]) == Vector2(0, 0));

	std::size_t initial_point_count = 0;
	{
		std::lock_guard<std::mutex> lock(context.game_node->get_progress_mutex());
		initial_point_count = context.game_node->get_delaunay_triangulation().pointCount();
	}
	int64_t const first_request = context.action_node->place_building("tallbot", Vector2(0.4, 0.4), 1);
	int64_t const second_request = context.action_node->place_building("tallbot", Vector2(0.49, 0.49), 1);
	CHECK(first_request > 0);
	CHECK(second_request > first_request);
	context.game_node->tick(2);

	flecs::entity placed_building;
	int building_count = 0;
	bool snapped_to_grid = false;
	{
		std::lock_guard<std::mutex> lock(context.game_node->get_progress_mutex());
		flecs::world &ecs = context.game_node->get_world().ecs;
		ecs.query<Building const, octopus::Position const>().each(
			[&](flecs::entity entity, Building const &, octopus::Position const &position) {
				++building_count;
				placed_building = entity;
				snapped_to_grid = position.pos.x.to_int() == 0 && position.pos.y.to_int() == 0;
			});
		CHECK(!context.game_node->get_delaunay_triangulation().holeTriangles().empty());
	}
	CHECK(building_count == 1);
	CHECK(snapped_to_grid);

	Array occupied_cells = context.action_node->get_building_occupied_cells(Vector2i(-2, -2), Vector2i(2, 2));
	CHECK(!occupied_cells.is_empty());
	bool found_center_cell = false;
	for (int i = 0; i < occupied_cells.size(); ++i) {
		if (Vector2i(occupied_cells[i]) == Vector2i(0, 0)) {
			found_center_cell = true;
			break;
		}
	}
	CHECK(found_center_cell);
	Dictionary occupied_preview = context.action_node->get_building_placement_preview("tallbot", Vector2(0.4, 0.4));
	CHECK(!occupied_preview["valid"].booleanize());

	{
		std::lock_guard<std::mutex> lock(context.game_node->get_progress_mutex());
		placed_building.destruct();
		CHECK(context.game_node->get_delaunay_triangulation().holeTriangles().empty());
		CHECK(context.game_node->get_delaunay_triangulation().pointCount() == initial_point_count);
	}

	CHECK(context.action_node->get_building_occupied_cells(Vector2i(-2, -2), Vector2i(2, 2)).is_empty());
	Dictionary cleared_preview = context.action_node->get_building_placement_preview("tallbot", Vector2(0.4, 0.4));
	CHECK(cleared_preview["valid"].booleanize());
}
