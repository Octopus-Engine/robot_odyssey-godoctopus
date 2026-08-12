
#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/window.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "testing/GameNodeBasic.test.h"
#include "testing/utils/GameNodeTestContextWithCustomPrefab.h"
#include "testing/utils/ModRuneDataHelper.h"

static Ref<godot::UnitPrefab> create_default_prefab(std::string name = "rambot") {
	auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
	prefab->set_prefab_name(name.c_str());
	prefab->set_hitpoint(1000);
	prefab->set_windup_x10(1);
	prefab->set_speed(10); // should set reload to 500 tick (formula is 5000/speed)
	prefab->set_range_x10(30);
	prefab->set_shield(0);
	prefab->set_damage(0);
	return prefab;
}

struct RuneNameData {
	String const rune_name;
	Dictionary const rune_data;
};

static double run_single_unit_multi_rune_stat(
		std::vector<RuneNameData> const &runes,
		void (*prefab_setup)(Ref<godot::UnitPrefab> &),
		double (*read_stat)(Ref<godot::InfoProxyResource> const &)) {
	Ref<godot::UnitPrefab> prefab = create_default_prefab();
	prefab_setup(prefab);
	GameNodeTestContextWithCustomPrefab context(prefab);

	Ref<godot::EntityGroup> group = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group("rambot", Vector2(100, 100), 0, 1, group);
	context.game_node->tick();

	for (auto const &rune : runes) {
		context.action_node->mod_rune("rambot", rune.rune_name, 0, rune.rune_data, true);
	}
	context.game_node->tick();
	context.game_node->tick();

	auto proxies = context.proxy_node->get_proxy_from_group(group);
	CHECK(proxies.size() == 1);
	if (proxies.size() != 1) {
		return -1;
	}
	Ref<godot::InfoProxyResource> proxy = proxies[0];
	return read_stat(proxy);
}

static void setup_damage_upgrade_prefab(Ref<godot::UnitPrefab> &prefab) {
	prefab->set_damage(0);
	prefab->set_special_x10(100); // special = 10
}

static double read_damage(Ref<godot::InfoProxyResource> const &proxy) { return proxy->get_damage(); }

void test_gamenode_stats_modifier_rune() {
	const double damage_without_upgrade = run_single_unit_multi_rune_stat(
			{
				{"RuneStats1", RuneDataBuilder()
					.set_base(10)
					.setType(godoctopus::StatsType::Damage)
					.setMechanicalPowerCoefPercent(10)
					.setModifierPriority(1)
					.build()}
			},
			setup_damage_upgrade_prefab,
			read_damage);
	const double damage_with_upgrade = run_single_unit_multi_rune_stat(
			{
				{"RuneStats1", RuneDataBuilder()
					.set_base(10)
					.setType(godoctopus::StatsType::Damage)
					.setMechanicalPowerCoefPercent(10)
					.setModifierPriority(1)
					.build()},
				{"RuneStats2", RuneDataBuilder()
					.set_base(100)
					.setType(godoctopus::StatsType::MechanicalPower)
					.setModifierPriority(0)
					.build()},

			},
			setup_damage_upgrade_prefab,
			read_damage);

	const double damage_with_double_upgrade = run_single_unit_multi_rune_stat(
			{
				{"RuneStats1", RuneDataBuilder()
					.set_base(10)
					.setType(godoctopus::StatsType::Damage)
					.setMechanicalPowerCoefPercent(10)
					.setModifierPriority(2)
					.build()},
				{"RuneStats2", RuneDataBuilder()
					.set_base(100)
					.setType(godoctopus::StatsType::MechanicalPower)
					.setAffinityCoefPercent(100)
					.setModifierPriority(1)
					.build()},
				{"RuneStats3", RuneDataBuilder()
					.set_base(100)
					.setType(godoctopus::StatsType::Affinity)
					.setModifierPriority(0)
					.build()},

			},
			setup_damage_upgrade_prefab,
			read_damage);

	// 100 mechanical power by default
	CHECK(damage_without_upgrade == 20); // 100 * 10% + 10 = 20
	CHECK(damage_with_upgrade == 30); // 200 * 10% + 10 = 30
	// 100 affinity from rune 3, so rune 2's mechanical power is increased by 100 (100% affinity), so total mechanical power is 300
	CHECK(damage_with_double_upgrade == 40); // 300 * 10% + 10 = 40
}
