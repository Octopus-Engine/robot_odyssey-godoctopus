
#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "testing/ActionNodeBasic.test.h"

struct ActionNodeTestContext {
	godot::GameNode *game_node = nullptr;
	godot::ActionNode *action_node = nullptr;
	godot::InfoProxyNode *proxy_node = nullptr;

	ActionNodeTestContext() {

		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");
		auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		prefab->set_prefab_name("gunbot");
		game_node->get_unit_prefabs().push_back(prefab);

		action_node = memnew(godot::ActionNode);
		action_node->set_ref_game_node(NodePath("/root/GameNode"));
		game_node->add_child(action_node);

		proxy_node = memnew(godot::InfoProxyNode);
		proxy_node->set_ref_game_node(NodePath("/root/GameNode"));
		proxy_node->set_refresh_tick(1);
		game_node->add_child(proxy_node);

		SceneTree::get_singleton()->get_root()->add_child(game_node);

		game_node->init_from_level(Dictionary());
	}

	~ActionNodeTestContext() {}
};

void test_actionnode_kill_dummy() {
	ActionNodeTestContext context;

	StringName unit_name = "gunbot";
	Ref<godot::EntityGroup> group(memnew(godot::EntityGroup));
	context.action_node->spawn_dummy_units_in_group(unit_name, Vector2(10, 20), 1, 1, group);
	context.game_node->tick();
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 1);
	}
	// Kill the dummy unit
	context.action_node->kill_units(group);
	context.game_node->tick();

	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 0);
	}
}

void test_actionnode_kill_non_dummy() {
	ActionNodeTestContext context;

	StringName unit_name = "gunbot";
	Ref<godot::EntityGroup> group(memnew(godot::EntityGroup));
	context.action_node->spawn_units_in_group(unit_name, Vector2(10, 20), 1, 1, group);
	context.game_node->tick();
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 1);
	}
	// Kill the non-dummy unit
	context.action_node->kill_units(group);
	// Need two ticks to make unit removal effective
	context.game_node->tick(2);

	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;
		CHECK(proxy_map.size() == 0);
	}
}
