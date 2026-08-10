#include "tests/test_macros.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "godoctopus/game/prefabs/UnitPrefab.h"

#include "testing/UndyingRune.test.h"
#include "testing/utils/GameNodeTestContextWithCustomPrefab.h"
#include "testing/utils/ModRuneDataHelper.h"

// Test 1: Verify UndyingBuff is applied when rune load reaches 5
void test_gamenode_undying_rune_applies_buff_at_rune_load_5() {
	// 1. Create prefab with basic stats
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_hitpoint(100);
	prefab->set_damage(10);
	prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	prefab->set_range_x10(300);
	prefab->set_windup_x10(1);

	// 2. Setup test context
	GameNodeTestContextWithCustomPrefab context(prefab);
	StringName unit_name = "rambot";

	// 3. Spawn unit
	Ref<godot::EntityGroup> group0 = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group0);
	context.game_node->tick();

	context.action_node->spawn_units_attack_move(unit_name, Vector2(102, 100), 1, 5, Vector2(100, 100));
	context.game_node->tick();

	// 4. Verify unit doesn't have UndyingBuff initially
	// (Note: We can't directly check for buff presence without additional infrastructure,
	// but we verify the unit is alive and has normal HP)
	auto proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	double initial_hp = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(initial_hp == 100);  // Should have full HP

	// 5. Apply ApplyUndyingBuffOnRuneLoad & AddRuneLoadOnHit
	context.action_node->mod_rune(unit_name, "ApplyUndyingBuffOnRuneLoad", 0, create_rune_data(), true);
	// Apply AddRuneLoadOnHit to enable rune load accumulation on attack
	context.action_node->mod_rune(unit_name, "AddRuneLoadOnHit", 0, create_rune_data(), true);
	context.game_node->tick();

	// 6. Attack the unit with another unit to increase rune load
	// Run for 20 ticks
	context.game_node->tick(20);

	// 7. Once we have 5 runes loaded, the UndyingBuff should be applied
	// The buff will consume 5 runes and apply itself for 15 seconds
	// For now, we just verify the unit has 1 hitpoint (undying buff should prevent death) and is alive
	proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	double hp_undying = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(hp_undying == 1);  // Unit should still be alive
	CHECK(Ref<godot::InfoProxyResource>(proxy_resources[0])->get_alive() == true);
}

// Test 2: Verify UndyingBuff expires after 15 seconds (750 ticks at 50Hz)
void test_gamenode_undying_buff_expires_after_15_seconds() {
	// 1. Create prefab with basic stats
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_hitpoint(60);
	prefab->set_damage(1);
	prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	prefab->set_range_x10(300);
	prefab->set_windup_x10(1);

	auto other_prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	other_prefab->set_hitpoint(1000);
	other_prefab->set_prefab_name("bladebot");
	other_prefab->set_damage(100);
	other_prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	other_prefab->set_range_x10(300);
	other_prefab->set_windup_x10(1);

	// 2. Setup test context
	GameNodeTestContextWithCustomPrefab context(prefab, other_prefab);
	StringName unit_name = "rambot";

	// 3. Spawn unit
	Ref<godot::EntityGroup> group0 = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group0);
	context.game_node->tick();

	context.action_node->spawn_units_attack_move("bladebot", Vector2(102, 100), 1, 5, Vector2(100, 100));
	context.game_node->tick();

	// 4. Verify unit doesn't have UndyingBuff initially
	// (Note: We can't directly check for buff presence without additional infrastructure,
	// but we verify the unit is alive and has normal HP)
	auto proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	double initial_hp = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(initial_hp == 60);  // Should have full HP

	// 5. Apply ApplyUndyingBuffOnRuneLoad & AddRuneLoadOnHit
	context.action_node->mod_rune(unit_name, "ApplyUndyingBuffOnRuneLoad", 0, create_rune_data(), true);
	// Apply AddRuneLoadOnHit to enable rune load accumulation on attack
	context.action_node->mod_rune(unit_name, "AddRuneLoadOnHit", 0, create_rune_data(), true);

	// 6. Attack the unit with another unit to increase rune load
	context.game_node->tick(20);

	// 7. Wait for buff expiration
	context.game_node->tick(850);

	// 8. Unit should have died because buff should not be active anymore
	// and should not have been reapplied
	proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 0);
}

// Test 3: Verify undying buff is reapplied after cooldown expires
void test_gamenode_undying_buff_reapplied_after_cooldown_expires() {
	// 1. Create prefab with basic stats
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_hitpoint(60);
	prefab->set_damage(10);
	prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	prefab->set_range_x10(300);
	prefab->set_windup_x10(1);

	auto other_prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	other_prefab->set_hitpoint(50);
	other_prefab->set_prefab_name("bladebot");
	other_prefab->set_damage(10);
	other_prefab->set_speed(5000); // should set reload to 1 tick (formula is 5000/speed)
	other_prefab->set_range_x10(300);
	other_prefab->set_windup_x10(1);

	// 2. Setup test context
	GameNodeTestContextWithCustomPrefab context(prefab, other_prefab);
	StringName unit_name = "rambot";

	// 3. Spawn unit
	Ref<godot::EntityGroup> group0 = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group0);
	context.game_node->tick();

	context.action_node->spawn_units_attack_move("bladebot", Vector2(102, 100), 1, 5, Vector2(100, 100));
	context.game_node->tick();

	// 4. Verify unit doesn't have UndyingBuff initially
	// (Note: We can't directly check for buff presence without additional infrastructure,
	// but we verify the unit is alive and has normal HP)
	auto proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	double initial_hp = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(initial_hp == 60);  // Should have full HP

	// 5. Apply ApplyUndyingBuffOnRuneLoad & AddRuneLoadOnHit
	context.action_node->mod_rune(unit_name, "ApplyUndyingBuffOnRuneLoad", 0, create_rune_data(), true);
	// Apply AddRuneLoadOnHit to enable rune load accumulation on attack
	context.action_node->mod_rune(unit_name, "AddRuneLoadOnHit", 0, create_rune_data(), true);

	// 6. Attack the unit with another unit to increase rune load
	context.game_node->tick(20);

	// 7. Wait for buff expiration
	context.game_node->tick(850);

	// 8. Unit should have died because buff should not be active anymore
	// and should still be alive because it killed all enemies
	proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	double hp_undying = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(hp_undying == 1);  // Unit should still be alive
	CHECK(Ref<godot::InfoProxyResource>(proxy_resources[0])->get_alive() == true);

	// For the test reset hp to max and wait for 15 seconds more to wait for cooldown to expire
	group0->get_entities()[0].set<octopus::HitPoint>({60});  // Set HP to max
	context.game_node->tick();

	proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	hp_undying = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(hp_undying == 60);  // Unit should be max hp

	// 8. Wait for cooldown expiration
	context.game_node->tick(750);

	context.action_node->spawn_units_attack_move("bladebot", Vector2(102, 100), 1, 5, Vector2(100, 100));
	context.game_node->tick();

	// 9. Attack the unit with another unit to increase rune load
	context.game_node->tick(20);

	// 10. Unit should have died because buff should not be active anymore
	// and should still be alive because it killed all enemies
	proxy_resources = context.proxy_node->get_proxy_from_group(group0);
	CHECK(proxy_resources.size() == 1);
	hp_undying = Ref<godot::InfoProxyResource>(proxy_resources[0])->get_hp();
	CHECK(hp_undying == 1);  // Unit should still be alive
	CHECK(Ref<godot::InfoProxyResource>(proxy_resources[0])->get_alive() == true);
}
