
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

static double run_single_unit_rune_stat(
		String const &rune_name,
		Dictionary const &rune_data,
		void (*prefab_setup)(Ref<godot::UnitPrefab> &),
		double (*read_stat)(Ref<godot::InfoProxyResource> const &)) {
	Ref<godot::UnitPrefab> prefab = create_default_prefab();
	prefab_setup(prefab);
	GameNodeTestContextWithCustomPrefab context(prefab);

	Ref<godot::EntityGroup> group = memnew(godot::EntityGroup);
	context.action_node->spawn_units_in_group("rambot", Vector2(100, 100), 0, 1, group);
	context.game_node->tick();

	context.action_node->mod_rune("rambot", rune_name, 0, rune_data, true);
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

static void setup_armor_upgrade_prefab(Ref<godot::UnitPrefab> &prefab) {
	prefab->set_shield(0);
	prefab->set_special_x10(100); // special = 10
}

static void setup_reload_upgrade_prefab(Ref<godot::UnitPrefab> &prefab) {
	prefab->set_speed(10); // should set reload to 500 tick (formula is 5000/speed)
	prefab->set_special_x10(100); // special = 10
}

static double read_damage(Ref<godot::InfoProxyResource> const &proxy) { return proxy->get_damage(); }
static double read_armor(Ref<godot::InfoProxyResource> const &proxy) { return proxy->get_armor(); }
static double read_reload(Ref<godot::InfoProxyResource> const &proxy) { return proxy->get_reload_time(); }

void test_gamenode_damage_buff_rune_special_upgrade_affects_bonus() {
	const double damage_without_upgrade = run_single_unit_rune_stat(
			"DamageBuffRuneSpecial",
			create_rune_data(1, 0, 10, 0, 0, 0, 0),
			setup_damage_upgrade_prefab,
			read_damage);
	const double damage_with_upgrade = run_single_unit_rune_stat(
			"DamageBuffRuneSpecial",
			create_rune_data(1, 0, 10, 0, 2, 0, 0),
			setup_damage_upgrade_prefab,
			read_damage);

	CHECK(damage_without_upgrade == 10);
	CHECK(damage_with_upgrade == 30);
}

void test_gamenode_armor_buff_rune_special_upgrade_affects_bonus() {
	const double armor_without_upgrade = run_single_unit_rune_stat(
			"ArmorBuffRuneSpecial",
			create_rune_data(1, 0, 10, 0, 0, 0, 0),
			setup_armor_upgrade_prefab,
			read_armor);
	const double armor_with_upgrade = run_single_unit_rune_stat(
			"ArmorBuffRuneSpecial",
			create_rune_data(1, 0, 10, 0, 2, 0, 0),
			setup_armor_upgrade_prefab,
			read_armor);

	CHECK(armor_without_upgrade == 1);
	CHECK(armor_with_upgrade == 3);
}

void test_gamenode_reload_buff_rune_special_upgrade_affects_bonus() {
	const double reload_without_upgrade = run_single_unit_rune_stat(
			"ReloadBuffRuneSpecial",
			create_rune_data(1, 0, 10, 0, 0, 0, 0),
			setup_reload_upgrade_prefab,
			read_reload);
	const double reload_with_upgrade = run_single_unit_rune_stat(
			"ReloadBuffRuneSpecial",
			create_rune_data(1, 0, 10, 0, 2, 0, 0),
			setup_reload_upgrade_prefab,
			read_reload);

	CHECK(reload_without_upgrade == 9.8);
	CHECK(reload_with_upgrade == 9.4);
}

void test_gamenode_damage_buff_rune_regular_flat_buff() {
	const double damage = run_single_unit_rune_stat(
			"DamageBuffRuneRegular",
			create_rune_data(1, 42, 0, 0, 0, 0, 0),
			setup_damage_upgrade_prefab,
			read_damage);
	CHECK(damage == 42);
}

void test_gamenode_conditional_damage_buff_high_life_tier1() {
	Ref<godot::UnitPrefab> prefab = create_default_prefab();
	prefab->set_damage(0);
	prefab->set_special_x10(100); // special = 10

	GameNodeTestContextWithCustomPrefab context(prefab);
	Ref<godot::EntityGroup> group = memnew(godot::EntityGroup);

	context.action_node->spawn_units_in_group("rambot", Vector2(100, 100), 0, 1, group);
	context.game_node->tick();

	context.action_node->mod_rune("rambot", "ConditionalDamageBuffHighLifeRuneTier1", 0, create_rune_data(1, 0, 18, 0, 0, 0, 0), true);
	context.game_node->tick();

	double damage_high_life = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group)[0])->get_damage();
	CHECK(damage_high_life == 18);

	group->get_entities()[0].set<octopus::HitPoint>({100}); // 10% hp, condition should be inactive
	context.game_node->tick();

	double damage_low_life = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(group)[0])->get_damage();
	CHECK(damage_low_life == 0);
}

void test_gamenode_apply_armor_buff_area_on_rune_load() {
	Ref<godot::UnitPrefab> prefab = create_default_prefab();
	Ref<godot::UnitPrefab> prefab_2 = create_default_prefab("gunbot");
	prefab->set_speed(5000); // 5000 ticks
	prefab->set_damage(10); // Ensure attacks register damage and rune loads increase.

	GameNodeTestContextWithCustomPrefab context(prefab, prefab_2);
	StringName unit_name = "rambot";
	StringName unit_name_2 = "gunbot";

	std::vector<Ref<godot::EntityGroup>> groups;
	// Source (team 0), enemy target (team 1), ally in range (team 0), ally out of range (team 0).
	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(100, 100), 0, 1, groups.back());
	context.game_node->tick();

	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(102, 100), 1, 1, groups.back());
	context.game_node->tick();

	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name_2, Vector2(103, 100), 0, 1, groups.back());
	context.game_node->tick();

	groups.push_back(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(120, 120), 0, 1, groups.back());
	context.game_node->tick();

	context.action_node->mod_rune("rambot", "ApplyArmorBuffAreaOnRuneLoad", 0, create_rune_data(1, 0, 10, 0, 0, 5, 500), true);
	context.action_node->mod_rune("rambot", "AddRuneLoadOnAttack", 0, create_rune_data(), true);
	context.game_node->tick();

	for (int i = 0; i < 100; ++i) {
		context.game_node->tick();
	}

	double armor[4];
	for (size_t i = 0; i < groups.size(); ++i) {
		armor[i] = Ref<godot::InfoProxyResource>(context.proxy_node->get_proxy_from_group(groups[i])[0])->get_armor();
	}

	CHECK(armor[0] == 1); // source gets the buff
	CHECK(armor[2] == 1); // in-range ally gets the buff
	CHECK(armor[1] == 0); // enemy does not get ally area buff
	CHECK(armor[3] == 0); // out-of-range ally does not get buff
}
