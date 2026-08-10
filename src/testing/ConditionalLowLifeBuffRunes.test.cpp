
#include "tests/test_macros.h"
#include "testing/ConditionalLowLifeBuffRunes.test.h"
#include "testing/utils/GameNodeTestContextWithCustomPrefab.h"
#include "testing/utils/ModRuneDataHelper.h"

void test_gamenode_conditional_armor_buff_low_life_tier1() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_shield(0);
	prefab->set_hitpoint(1000);  // Max HP
	prefab->set_windup_x10(1);
	prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	prefab->set_range_x10(30);
	prefab->set_special_x10(100);  // 10 special for scaling (x10)

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn a unit with high HP - conditional buff should NOT be active
	Ref<godot::EntityGroup> group_high_hp = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group_high_hp);
	context.game_node->tick();

	// Apply ConditionalArmorBuffLowLifeRuneTier1 rune with level 1
	context.action_node->mod_rune("rambot", "ConditionalArmorBuffLowLifeRuneTier1", 0, create_rune_data(1, 0, 40, 0, 0, 0, 0), true);
	context.game_node->tick();

	// Verify armor is 0 at high HP (buff should not be active)
	double armor_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_armor();
	CHECK(armor_high_hp == 0);  // Buff inactive at high HP

	// Modify entity hp manually to lower it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({100});  // Set HP to 40% to activate buff
	context.game_node->tick();

	// Verify armor is more than 0 at low HP (buff should be active)
	armor_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_armor();
	CHECK(armor_high_hp == 4);  // Buff active at low HP (2 base + 0.2 * 10 special = 4 total)

	// Modify entity hp manually to increase it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({1000});  // Set HP to 100% to deactivate buff
	context.game_node->tick();

	// Verify armor is 0 at high HP (buff should not be active)
	armor_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_armor();
	CHECK(armor_high_hp == 0);  // Buff inactive at high HP
}

void test_gamenode_conditional_damage_buff_low_life_tier1() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_damage(0);  // Start with 0 damage
	prefab->set_hitpoint(1000);  // Max HP
	prefab->set_windup_x10(1);
	prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	prefab->set_range_x10(30);
	prefab->set_special_x10(100);  // 10 special for scaling (x10)

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn a unit with high HP - conditional buff should NOT be active
	Ref<godot::EntityGroup> group_high_hp = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group_high_hp);
	context.game_node->tick();

	// Apply ConditionalDamageBuffLowLifeRuneTier1 rune with level 1
	context.action_node->mod_rune("rambot", "ConditionalDamageBuffLowLifeRuneTier1", 0, create_rune_data(1, 0, 18, 0, 0, 0, 0), true);
	context.game_node->tick();

	// Verify damage is 0 at high HP (buff should not be active)
	double damage_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_damage();
	CHECK(damage_high_hp == 0);  // Buff inactive at high HP

	// Modify entity hp manually to lower it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({100});  // Set HP to 40% to activate buff
	context.game_node->tick();

	// Verify damage is more than 0 at low HP (buff should be active)
	damage_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_damage();
	CHECK(damage_high_hp == 18);  // Buff active at low HP (8 base + 0.1 * 10 special = 18 total)

	// Modify entity hp manually to increase it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({1000});  // Set HP to 100% to deactivate buff
	context.game_node->tick();

	// Verify damage is 0 at high HP (buff should not be active)
	damage_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_damage();
	CHECK(damage_high_hp == 0);  // Buff inactive at high HP
}

void test_gamenode_conditional_reload_buff_low_life_tier1() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_damage(1);
	prefab->set_hitpoint(1000);  // Max HP
	prefab->set_windup_x10(1);
	prefab->set_speed(10); // should set reload to 500 tick (formula is 5000/speed)
	prefab->set_range_x10(30);
	prefab->set_special_x10(100);  // 10 special for scaling (x10)

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn a unit with high HP - conditional buff should NOT be active
	Ref<godot::EntityGroup> group_high_hp = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group_high_hp);
	context.game_node->tick();

	// Apply ConditionalReloadBuffLowLifeRuneTier1 rune with level 1
	context.action_node->mod_rune("rambot", "ConditionalReloadBuffLowLifeRuneTier1", 0, create_rune_data(1, 0, 40, 0, 0, 0, 0), true);
	context.game_node->tick();

	// Verify damage is 0 at high HP (buff should not be active)
	double reload_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_reload_time();
	CHECK(reload_high_hp == 10);  // Buff inactive at high HP

	// Modify entity hp manually to lower it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({100});  // Set HP to 40% to activate buff
	context.game_node->tick();

	// Verify damage is more than 0 at low HP (buff should be active)
	reload_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_reload_time();
	CHECK(reload_high_hp == 9.2);  // Buff active at low HP (Reduction is 20 ticks + 2 * 10 special -> 40 total which is 0.8 seconds)

	// Modify entity hp manually to increase it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({1000});  // Set HP to 100% to deactivate buff
	context.game_node->tick();

	// Verify damage is 0 at high HP (buff should not be active)
	reload_high_hp = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_reload_time();
	CHECK(reload_high_hp == 10);  // Buff inactive at high HP
}
