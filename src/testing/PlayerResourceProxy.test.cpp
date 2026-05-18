#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/window.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/game/prefabs/UnitPrefab.h"
#include "godoctopus/proxy/PlayerResourceProxyNode.h"

#include "octopus/components/basic/player/PlayerUpgrade.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus/world/resources/ResourceInfo.hh"
#include "octopus/world/resources/ResourceStock.hh"

#include "testing/PlayerResourceProxy.test.h"

struct PlayerResourceProxyTestContext {
	godot::GameNode *game_node = nullptr;
	godot::PlayerResourceProxyNode *proxy_node = nullptr;

	PlayerResourceProxyTestContext() {
		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");

		auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		prefab->set_prefab_name("gunbot");
		game_node->get_unit_prefabs().push_back(prefab);

		proxy_node = memnew(godot::PlayerResourceProxyNode);
		proxy_node->set_ref_game_node(NodePath("/root/GameNode"));
		proxy_node->set_refresh_tick(1);
		game_node->add_child(proxy_node);

		SceneTree::get_singleton()->get_root()->add_child(game_node);
		game_node->init_from_level(Dictionary());
	}
};

void test_player_resource_proxy_sync() {
	PlayerResourceProxyTestContext context;
	auto &ecs = context.game_node->get_world().ecs;

	flecs::entity player0 = ecs.entity("player0")
		.set<octopus::PlayerInfo>({0, 1})
		.set<octopus::ResourceStock>({})
		.set<octopus::PlayerUpgrade>({});
	player0.get_mut<octopus::ResourceStock>().resource["ore"] = octopus::ResourceInfo{octopus::Fixed(120), octopus::Fixed(300)};
	player0.get_mut<octopus::PlayerUpgrade>().upgrades["tier_1"] = 2;

	flecs::entity player1 = ecs.entity("player1")
		.set<octopus::PlayerInfo>({1, 2})
		.set<octopus::ResourceStock>({})
		.set<octopus::PlayerUpgrade>({});
	player1.get_mut<octopus::ResourceStock>().resource["crystal"] = octopus::ResourceInfo{octopus::Fixed(45), octopus::Fixed(60)};
	player1.get_mut<octopus::PlayerUpgrade>().upgrades["tier_2"] = 1;

	context.game_node->tick();

	Ref<godot::PlayerResourceProxyResource> proxy0 = context.proxy_node->get_proxy_from_player(0);
	CHECK(proxy0.is_valid());
	CHECK(proxy0->get_player() == 0);
	CHECK(proxy0->get_team() == 1);
	CHECK(proxy0->get_resource_amount("ore") == 120);
	CHECK(proxy0->get_resource_cap("ore") == 300);
	CHECK(proxy0->check_resource("ore", 120));
	CHECK(proxy0->check_upgrade("tier_1", 2));
	CHECK(!proxy0->check_upgrade("tier_1", 3));

	Ref<godot::PlayerResourceProxyResource> proxy1 = context.proxy_node->get_proxy_from_player(1);
	CHECK(proxy1.is_valid());
	CHECK(proxy1->get_resource_amount("crystal") == 45);
	Dictionary upgrade_requirements;
	upgrade_requirements["tier_2"] = 1;
	CHECK(proxy1->check_upgrades(upgrade_requirements));

	TypedArray<Ref<godot::PlayerResourceProxyResource>> all_players = context.proxy_node->get_proxy_from_players();
	CHECK(all_players.size() == 2);
}
