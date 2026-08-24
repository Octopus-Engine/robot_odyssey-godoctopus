#pragma once

#include "testing/GameNodeBasic.test.h"
#include "testing/Triggers.StatBuff.test.h"
#include "testing/Triggers.test.h"
#include "testing/ConditionalLowLifeBuffRunes.test.h"
#include "testing/SpawnCloneUnitRune.test.h"
#include "testing/runes/Rune.test.h"
#include "testing/CommandNodeProduction.test.h"
#include "testing/PlayerProxy.test.h"
#include "testing/StatsModifiers.test.h"
#include "testing/Rune.StatsModifier.test.h"

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

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode DamageOnDeath rune") {
	test_gamenode_damage_on_death_rune();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode AoePulseDamages rune") {
	test_gamenode_aoe_pulse_damage_based_on_hitpoint();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Lifesteal rune") {
	test_gamenode_lifesteal_rune();
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

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Stats Modifier Test") {
	test_gamenode_stats_modifier_rune();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode DamageBuffRuneSpecial upgrade scales bonus") {
	test_gamenode_damage_buff_rune_special_upgrade_affects_bonus();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode ArmorBuffRuneSpecial upgrade scales bonus") {
	test_gamenode_armor_buff_rune_special_upgrade_affects_bonus();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode ReloadBuffRuneSpecial upgrade scales bonus") {
	test_gamenode_reload_buff_rune_special_upgrade_affects_bonus();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode DamageBuffRuneRegular applies flat buff") {
	test_gamenode_damage_buff_rune_regular_flat_buff();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode ConditionalDamageBuffHighLifeRuneTier1") {
	test_gamenode_conditional_damage_buff_high_life_tier1();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode ApplyArmorBuffAreaOnRuneLoad") {
	test_gamenode_apply_armor_buff_area_on_rune_load();
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

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Command Production with two producers") {
	test_commandnode_production_two_producers_parallel_output();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] GameNode Player Resource Proxy sync") {
	test_player_proxy_sync();
}

TEST_CASE("[SceneTree][Node][Editor][godoctopus2] StatsModifierList tests") {
	test_stats_modifiers();
}

} // namespace godoctopus2_tests
