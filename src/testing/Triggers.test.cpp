
#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/window.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "testing/GameNodeBasic.test.h"

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

void test_gamenode_trigger_armor_buff() {
	// Create a no-damage unit prefab for both teams
	auto no_damage_prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	no_damage_prefab->set_prefab_name("rambot");
	no_damage_prefab->set_armor(0);
	no_damage_prefab->set_hitpoint(1000);  // Give sufficient HP for damage calculation
	no_damage_prefab->set_windup_x10(1);
	no_damage_prefab->set_reload_x10(1);
	no_damage_prefab->set_range_x10(30);

	GameNodeTestContextWithCustomPrefab context(no_damage_prefab);

	StringName unit_name = "rambot";

	// Spawn a unit for team 1 at position (100, 100) - the target
	context.action_node->spawn_units(unit_name, Vector2(100, 100), 1, 1);
	context.game_node->tick();

	// Spawn a unit for team 0 at position (102, 100) - very close to team 1 unit
	// Distance is ~2 units, which should be within AoE radius (range template parameter is 3)
	context.action_node->spawn_units(unit_name, Vector2(102, 100), 0, 1);
	context.game_node->tick();

	// Apply AoePulseDamageBasedOnHitpoint rune level 0 to the team 0 unit via ActionNode
	context.action_node->mod_rune("rambot", "ApplyArmorBuffOnRuneLoad", 0, 0, true);
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, 0, true);
	context.game_node->tick();

	// Tick the game multiple times to allow the AoE pulse to trigger
	// The rune deals damage periodically, so we need enough ticks for it to trigger
	for (int i = 0; i < 30; ++i) {
		context.game_node->tick();
	}

	// Get final HP values and verify the target took damage
	double armor = 0;
	auto locker = context.proxy_node->get_data_locker();
	auto const &proxy_map = locker.proxy_map;

	for (auto const &[entity_id, proxy_data] : proxy_map) {
		if (proxy_data.get_team() == 0) {
			armor = proxy_data.get_armor();
			std::cout<<"Final armor: " << armor << std::endl;
			std::cout<<"Rune loads: " << proxy_data.get_rune_loads() << std::endl;
		}
	}
	// Expected 1 armor from the buff
	CHECK(armor == 1);
}

void test_gamenode_trigger_damage_buff_area() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_damage_x10(0);  // Set base damage to 0 for testing
	prefab->set_hitpoint(1000);
	prefab->set_windup_x10(1);
	prefab->set_reload_x10(1);
	prefab->set_range_x10(30);

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn unit A at (100, 100) - team 0, will apply the rune and attack
	context.action_node->spawn_units(unit_name, Vector2(100, 100), 0, 1);
	context.game_node->tick();

	// Spawn a target at (102, 100) - team 1, for unit A to attack
	context.action_node->spawn_units(unit_name, Vector2(102, 100), 1, 1);
	context.game_node->tick();

	// Spawn unit B at (103, 100) - team 0, within area range (~3 units from A)
	context.action_node->spawn_units(unit_name, Vector2(103, 100), 0, 1);
	context.game_node->tick();

	// Spawn unit C at (112, 120) - team 0, outside area range (~14 units from A)
	context.action_node->spawn_units(unit_name, Vector2(112, 120), 0, 1);
	context.game_node->tick();

	// Apply ApplyDamageBuffAreaOnRuneLoad rune to rambot
	context.action_node->mod_rune("rambot", "ApplyDamageBuffAreaOnRuneLoad", 0, 0, true);
	// Apply AddRuneLoadOnAttack to enable rune load accumulation on attack
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, 0, true);
	context.game_node->tick();

	// Tick the game multiple times to allow unit A to attack and reach rune load 3
	for (int i = 0; i < 50; ++i) {
		context.game_node->tick();
	}

	// Verify buff is applied to team 0 units based on range
	double unit_a_damage = 0;
	double unit_b_damage = 0;
	double unit_c_damage = 0;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;

		std::vector<double> damages;
		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_team() == 0) {
				damages.push_back(proxy_data.get_damage());
			}
		}
		CHECK(damages.size() == 3);
		unit_a_damage = damages[0];
		unit_b_damage = damages[1];
		unit_c_damage = damages[2];
	}

	// Unit A (source at 100,100) and Unit B (at 103,100, within range 5) should have +20 damage buff
	CHECK(unit_a_damage == 20);
	CHECK(unit_b_damage == 20);
	// Unit C (at 112,120, ~14 units away, outside range 5) should not have the buff
	CHECK(unit_c_damage == 0);
}

void test_gamenode_trigger_attack_speed_debuff_area() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_damage_x10(0);  // Set base damage to 0 for testing
	prefab->set_hitpoint(1000);
	prefab->set_windup_x10(1); // 0.1 seconds = 5 ticks with tickrate 50
	prefab->set_reload_x10(1); // 0.1 seconds = 5 ticks with tickrate 50
	prefab->set_range_x10(30);

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn unit A at (100, 100) - team 0, will apply the debuff rune and attack
	context.action_node->spawn_units(unit_name, Vector2(101, 100), 0, 1);
	context.game_node->tick();

	// Spawn enemy E1 at (102, 100) - team 1, for unit A to attack (and be debuffed)
	context.action_node->spawn_units(unit_name, Vector2(102, 100), 1, 1);
	context.game_node->tick();

	// Spawn enemy E2 at (111, 120) - team 1, outside debuff area range (~14 units from A)
	context.action_node->spawn_units(unit_name, Vector2(111, 120), 1, 1);
	context.game_node->tick();

	// Spawn unit B at (103, 100) - team 0, ally within area range (~3 units from A)
	context.action_node->spawn_units(unit_name, Vector2(103, 100), 0, 1);
	context.game_node->tick();

	// Apply ApplyAttackSpeedDebuffAreaOnRuneLoad rune to rambot (all instances)
	context.action_node->mod_rune("rambot", "ApplyAttackSpeedDebuffAreaOnRuneLoad", 0, 0, true);
	// Apply AddRuneLoadOnAttack to enable rune load accumulation on attack
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, 0, true);
	context.game_node->tick();

	// Tick the game multiple times to allow unit A to attack and reach rune load 3
	for (int i = 0; i < 50; ++i) {
		context.game_node->tick();
	}

	// Verify rune load accumulation
	double unit_a_rune_load = 0;
	int unit_count = 0;

	auto locker = context.proxy_node->get_data_locker();
	auto const &proxy_map = locker.proxy_map;

	std::vector<double> enemy_reload_times;
	std::vector<double> ally_reload_times;
	for (auto const &[entity_id, proxy_data] : proxy_map) {
		unit_count++;
		if (proxy_data.get_team() == 0 && unit_a_rune_load == 0) {
			unit_a_rune_load = proxy_data.get_rune_loads();
		}
		if (proxy_data.get_team() == 1) {
			enemy_reload_times.push_back(proxy_data.get_reload_time());
		} else {
			ally_reload_times.push_back(proxy_data.get_reload_time());
		}
	}

	CHECK(enemy_reload_times[0] == 0.1); // Base reload time 5 which is 0.1 seconds with tickrate 50
	CHECK(enemy_reload_times[1] == 0.7); // Base reload time 35 (5 + 30 debuff) which is 0.7 seconds with tickrate 50
	CHECK(ally_reload_times[0] == 0.1); // Base reload time 5 which is 0.1 seconds with tickrate 50
	CHECK(ally_reload_times[1] == 0.1); // Base reload time 5 which is 0.1 seconds with tickrate 50

}
