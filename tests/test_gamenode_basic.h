#pragma once

#include "testing/GameNodeBasic.test.h"

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

} // namespace godoctopus2_tests
