#include "SpawnCloneUnitRune.test.h"

#include "tests/test_macros.h"

#include "testing/utils/GameNodeTestContextWithCustomPrefab.h"
#include "testing/utils/ModRuneDataHelper.h"

void test_gamenode_spawn_clone_unit_rune() {
	// Create a unit prefab for testing
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_hitpoint(100);  // Max HP
	prefab->set_prefab_name("rambot");

	GameNodeTestContextWithCustomPrefab context(prefab);

	StringName unit_name = "rambot";

	// Spawn a unit with high HP - conditional buff should NOT be active
	Ref<godot::EntityGroup> group_high_hp = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, group_high_hp);
	context.game_node->tick();

	// Apply SpawnCloneUnitRune rune
	context.action_node->mod_rune("rambot", "SpawnCloneUnitRune", 0, create_rune_data(), true);
	context.game_node->tick();

	// Verify armor is 0 at high HP (buff should not be active)
	bool is_alive = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group_high_hp)[0])->get_alive();
	CHECK(is_alive == true);

	// Modify entity hp manually to lower it
	group_high_hp->get_entities()[0].set<octopus::HitPoint>({0});  // Set HP to 0 to trigger death
	context.game_node->tick();

	auto group = context.proxy_node->get_proxy_from_group(group_high_hp);
	CHECK(group.size() == 0);

	flecs::entity new_unit;
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 1);

		// Check that a new unit has been spawned
		for (auto const &[entity_id, proxy_data] : proxy_map) {
			CHECK(proxy_data.get_hp() == 50);
			new_unit = proxy_data.entity;
		}
	}

	// Set hp to 0 to ensure no other unit is spawned
	new_unit.set<octopus::HitPoint>({0});
	context.game_node->tick();

	CHECK(new_unit.enabled() == false);

	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 0);
	}
}
