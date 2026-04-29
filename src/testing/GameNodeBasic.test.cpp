
#include "tests/test_macros.h"
#include "scene/main/node.h"
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

struct GameNodeTestContextWithCustomPrefab {
	godot::GameNode *game_node = nullptr;
	godot::ActionNode *action_node = nullptr;
	godot::InfoProxyNode *proxy_node = nullptr;

	GameNodeTestContextWithCustomPrefab(Ref<godot::UnitPrefab> custom_prefab) {

		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");
		game_node->get_unit_prefabs().push_back(custom_prefab);

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

	~GameNodeTestContextWithCustomPrefab() {}
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

void test_gamenode_aoe_pulse_damage_based_on_hitpoint() {
	// Create a no-damage unit prefab for both teams
	auto no_damage_prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	no_damage_prefab->set_prefab_name("rambot");
	no_damage_prefab->set_attack_enabled(false);
	no_damage_prefab->set_hitpoint(100);  // Give sufficient HP for damage calculation
	no_damage_prefab->set_range_x10(30);  // Set reasonable attack range

	GameNodeTestContextWithCustomPrefab context(no_damage_prefab);

	StringName unit_name = "rambot";

	// Spawn a unit for team 1 at position (100, 100) - the target
	context.action_node->spawn_units(unit_name, Vector2(100, 100), 1, 1);
	context.game_node->tick();

	// Spawn a unit for team 0 at position (102, 100) - very close to team 1 unit
	// Distance is ~2 units, which should be within AoE radius (range template parameter is 3)
	context.action_node->spawn_units(unit_name, Vector2(102, 100), 0, 1);
	context.game_node->tick();

	// Get initial HP values
	double team1_initial_hp = 0;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 2);

		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_team() == 1) {
				team1_initial_hp = proxy_data.get_hp();
			}
		}
	}

	// Apply AoePulseDamageBasedOnHitpoint rune level 0 to the team 0 unit via ActionNode
	context.action_node->mod_rune("rambot", "AoePulseDamageBasedOnHitpoint", 0, 0, true);
	context.game_node->tick();

	// Tick the game multiple times to allow the AoE pulse to trigger
	// The rune deals damage periodically, so we need enough ticks for it to trigger
	for (int i = 0; i < 10; ++i) {
		context.game_node->tick();
	}

	// Get final HP values and verify the target took damage
	double team1_final_hp = 0;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;

		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_team() == 1) {
				team1_final_hp = proxy_data.get_hp();
			}
		}
	}

	// Verify that team 1 (the target) took damage from the AoE effect
	// even though team 0 (the source) has zero base damage
	double team1_damage = team1_initial_hp - team1_final_hp;
	// Damage is 5% of 100 hp
	CHECK(team1_damage == 5);
}

