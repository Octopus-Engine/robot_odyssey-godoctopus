
#include "tests/test_macros.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "testing/GameNodeBasic.test.h"
#include "testing/utils/GameNodeTestContextWithCustomPrefab.h"
#include "testing/utils/ModRuneDataHelper.h"

void test_gamenode_lifesteal_rune() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name("rambot");
	prefab->set_damage(10);  // Set base damage to 10 for testing
	prefab->set_hitpoint(100);  // Set HP to 100 for testing
	prefab->set_mechanical_power(200); // Set power to 200 to increase damage taken by 10%
	prefab->set_mechanical_armor(100); // Set armor to 100 to increase damage taken by 10%
	prefab->set_windup_x10(1);
	prefab->set_speed(500); // should set reload to 10 ticks (formula is 5000/speed)
	prefab->set_range_x10(30);

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn a unit for team 0 at position (100, 100) - the attacker
	context.action_node->spawn_units(unit_name, Vector2(100, 100), 0, 1);
	context.game_node->tick();

	// Spawn a unit for team 1 at position (102, 100) - the target
	context.action_node->spawn_units(unit_name, Vector2(102, 100), 1, 1);
	context.game_node->tick();

	// Apply Lifesteal rune level 0 to the team 0 unit via ActionNode
	context.action_node->mod_rune("rambot", "LifestealRune", 0,
		RuneDataBuilder()
			.set_base(0)
			.setHitPointsCoefPercent(100) // Lifesteal is 100% of damage dealt
			.build()
		, true);

	// Tick the game multiple times to allow the attacker to attack and heal
	context.game_node->tick(35);

	// Get final HP values and verify the attacker healed from lifesteal
	double attacker_final_hp = 0;
	double target_final_hp = 0;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;

		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_team() == 0) {
				attacker_final_hp = proxy_data.get_hp();
			} else if (proxy_data.get_team() == 1) {
				target_final_hp = proxy_data.get_hp();
			}
		}
	}
	// Verify that the attacker healed from lifesteal
	CHECK(attacker_final_hp == 100);
	CHECK(target_final_hp == 89);
}
