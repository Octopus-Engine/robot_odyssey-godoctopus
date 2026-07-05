
#include "tests/test_macros.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "testing/GameNodeBasic.test.h"
#include "testing/utils/GameNodeTestContextWithCustomPrefab.h"
#include "testing/utils/ModRuneDataHelper.h"

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
	context.action_node->mod_rune("rambot", "AoePulseDamageBasedOnHitpoint", 0, create_rune_data(1, 0, 2, 1, 2, 0), true);
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
	// Damage is 2% of 100 hp
	CHECK(team1_damage == 2);
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
	context.action_node->mod_rune("rambot", "ApplyArmorBuffOnRuneLoad", 0, create_rune_data(1, 0, 10, 1, 0, 15), true);
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, create_rune_data(), true);
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

	std::vector<Ref<godot::EntityGroup>> groups;
	// Spawn unit A at (100, 100) - team 0, will apply the rune and attack
	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, groups.back());
	context.game_node->tick();

	// Spawn a target at (102, 100) - team 1, for unit A to attack
	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(102, 100), 1, 1, groups.back());
	context.game_node->tick();

	// Spawn unit B at (103, 100) - team 0, within area range (~3 units from A)
	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(103, 100), 0, 1, groups.back());
	context.game_node->tick();

	// Spawn unit C at (112, 120) - team 0, outside area range (~14 units from A)
	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(112, 120), 0, 1, groups.back());
	context.game_node->tick();

	// Apply ApplyDamageBuffAreaOnRuneLoad rune to rambot
	context.action_node->mod_rune("rambot", "ApplyDamageBuffAreaOnRuneLoad", 0, create_rune_data(1, 0, 20, 2, 5, 15), true);
	// Apply AddRuneLoadOnAttack to enable rune load accumulation on attack
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, create_rune_data(), true);
	context.game_node->tick();

	// Tick the game multiple times to allow unit A to attack and reach rune load 3
	for (int i = 0; i < 50; ++i) {
		context.game_node->tick();
	}


	double damage[4];
	for (size_t i = 0 ; i < groups.size() ; ++ i) {
		damage[i] = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(groups[i])[0])->get_damage();
	}

	// Unit A (source at 100,100) and Unit B (at 103,100, within range 5) should have +20 damage buff
	CHECK(damage[0] == 20);
	CHECK(damage[2] == 20);
	// Unit C (at 112,120, ~14 units away, outside range 5) should not have the buff
	CHECK(damage[3] == 0);
	// enemy unnafected
	CHECK(damage[1] == 0);
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
	Ref<godot::EntityGroup> groupA = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(101, 100), 0, 1, groupA);
	context.game_node->tick();

	// Spawn enemy E1 at (102, 100) - team 1, for unit A to attack (and be debuffed)
	Ref<godot::EntityGroup> groupB = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(102, 100), 1, 1, groupB);
	context.game_node->tick();

	// Spawn enemy E2 at (111, 120) - team 1, outside debuff area range (~14 units from A)
	Ref<godot::EntityGroup> groupC = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(111, 120), 1, 1, groupC);
	context.game_node->tick();

	// Spawn unit B at (103, 100) - team 0, ally within area range (~3 units from A)
	Ref<godot::EntityGroup> groupD = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(103, 100), 0, 1, groupD);
	context.game_node->tick();

	// Apply ApplyAttackSpeedDebuffAreaOnRuneLoad rune to rambot (all instances)
	context.action_node->mod_rune("rambot", "ApplyAttackSpeedDebuffAreaOnRuneLoad", 0, create_rune_data(1, 0, -30, 0, 5, 500), true);
	// Apply AddRuneLoadOnAttack to enable rune load accumulation on attack
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, create_rune_data(), true);
	context.game_node->tick();

	// Tick the game multiple times to allow unit A to attack and reach rune load 3
	for (int i = 0; i < 50; ++i) {
		context.game_node->tick();
	}

	double reload_times[4];
	reload_times[0] = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(groupA)[0])->get_reload_time();
	reload_times[1] = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(groupB)[0])->get_reload_time();
	reload_times[2] = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(groupC)[0])->get_reload_time();
	reload_times[3] = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(groupD)[0])->get_reload_time();

	CHECK(reload_times[0] == 0.1); // Base reload time 5 which is 0.1 seconds with tickrate 50 (ally)
	CHECK(reload_times[1] == 0.7); // Base reload time 5 which is 0.1 seconds with tickrate 50 (enenmy inrange)
	CHECK(reload_times[2] == 0.1); // Base reload time 35 (5 + 30 debuff) which is 0.7 seconds with tickrate 50 (enemy out of range)
	CHECK(reload_times[3] == 0.1); // Base reload time 5 which is 0.1 seconds with tickrate 50 (ally)
}
