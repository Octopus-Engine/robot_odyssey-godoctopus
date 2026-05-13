#pragma once

#include "testing/GameNodeBasic.test.h"
#include "testing/Triggers.test.h"
#include "testing/ConditionalLowLifeBuffRunes.test.h"
#include "testing/SpawnCloneUnitRune.test.h"
#include "testing/UndyingRune.test.h"
#include "testing/CommandNodeProduction.test.h"

namespace godoctopus2_tests {

/*
 * Basic GameNode functionality tests
 * These tests verify that GameNode can be instantiated and initialized
 * without crashing, and that basic world setup works correctly.
 */


TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode instantiation") {
	test_gamenode_basic();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode units take damage") {
	test_gamenode_units_take_damage();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode AoePulseDamageBasedOnHitpoint rune") {
	test_gamenode_aoe_pulse_damage_based_on_hitpoint();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Trigger Armor Buff rune") {
	test_gamenode_trigger_armor_buff();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Trigger Damage Buff Area rune") {
	test_gamenode_trigger_damage_buff_area();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Trigger Attack Speed Debuff Area rune") {
	test_gamenode_trigger_attack_speed_debuff_area();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Conditional Armor Buff Low Life Tier1 rune") {
	test_gamenode_conditional_armor_buff_low_life_tier1();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Conditional Damage Buff Low Life Tier1 rune") {
	test_gamenode_conditional_damage_buff_low_life_tier1();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Conditional Reload Buff Low Life Tier1 rune") {
	test_gamenode_conditional_reload_buff_low_life_tier1();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode SpawnCloneUnitRune rune") {
	test_gamenode_spawn_clone_unit_rune();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode UndyingRune applies buff at rune load 5") {
	test_gamenode_undying_rune_applies_buff_at_rune_load_5();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode UndyingRune expires after 15 seconds") {
	test_gamenode_undying_buff_expires_after_15_seconds();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode UndyingRune buff reapplied after cooldown expires") {
	test_gamenode_undying_buff_reapplied_after_cooldown_expires();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Command Production with producer") {
	test_commandnode_production_with_producer();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Command Production without producer") {
	test_commandnode_production_without_producer();
}

} // namespace godoctopus2_tests
