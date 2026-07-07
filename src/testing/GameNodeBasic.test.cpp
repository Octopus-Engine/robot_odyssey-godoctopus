
#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "testing/GameNodeBasic.test.h"

struct GameNodeTestContext {
	godot::GameNode *game_node = nullptr;
	godot::ActionNode *action_node = nullptr;
	godot::InfoProxyNode *proxy_node = nullptr;

	GameNodeTestContext() {

		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");
		auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		prefab->set_prefab_name("gunbot");
		game_node->get_unit_prefabs().push_back(prefab);

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

	~GameNodeTestContext() {}
};

void test_gamenode_basic() {
	GameNodeTestContext context;

	StringName unit_name = "gunbot";
	context.action_node->spawn_units(unit_name, Vector2(10, 20), 1, 5);
	context.game_node->tick();
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 5);
	}
}

void test_gamenode_units_take_damage() {
	GameNodeTestContext context;

	StringName unit_name = "gunbot";

	// Spawn a unit for team 1 at position (10, 20)
	context.action_node->spawn_units(unit_name, Vector2(10, 20), 1, 1);
	context.game_node->tick();

	// Spawn a unit for team 0 at position (12, 20) with attack_move towards team 1's unit
	context.action_node->spawn_units_attack_move(unit_name, Vector2(12, 20), 0, 1, Vector2(10, 20));
	context.game_node->tick();

	// Get initial HP values
	double team1_initial_hp = 0;
	double team2_initial_hp = 0;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 2);

		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_team() == 1) {
				team1_initial_hp = proxy_data.get_hp();
			} else if (proxy_data.get_team() == 0) {
				team2_initial_hp = proxy_data.get_hp();
			}
		}
	}

	// Tick the game multiple times to allow combat to occur
	for (int i = 0; i < 200; ++i) {
		context.game_node->tick();
	}

	// Get final HP values and verify they decreased
	double team1_final_hp = 0;
	double team2_final_hp = 0;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;

		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_team() == 1) {
				team1_final_hp = proxy_data.get_hp();
			} else if (proxy_data.get_team() == 0) {
				team2_final_hp = proxy_data.get_hp();
			}
		}
	}

	// Verify that at least one unit took damage
	double total_damage = (team1_initial_hp - team1_final_hp) + (team2_initial_hp - team2_final_hp);
	CHECK(total_damage > 0);
}
