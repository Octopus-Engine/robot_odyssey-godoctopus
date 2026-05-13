#include "tests/test_macros.h"
#include "scene/main/node.h"
#include "scene/main/window.h"

#include "godoctopus/game/GameNode.h"
#include "godoctopus/command/CommandNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "godoctopus/game/prefabs/UnitPrefab.h"
#include "godoctopus/components/types/Types.h"

#include "octopus/components/basic/player/Team.hh"
#include "octopus/components/advanced/production/queue/ProductionQueue.hh"
#include "octopus/world/player/PlayerInfo.hh"
#include "octopus_types.h"

#include "testing/CommandNodeProduction.test.h"

static int count_entities_for_player(godot::InfoProxyNode *proxy_node, int player) {
	auto locker = proxy_node->get_data_locker();
	auto const &proxy_map = locker.proxy_map;

	int count = 0;
	for (auto const &[entity_id, proxy_data] : proxy_map) {
		if (proxy_data.get_player() == player) {
			++count;
		}
	}
	return count;
}

struct GameNodeTestContextWithProduction {
	godot::GameNode *game_node = nullptr;
	godot::CommandNode *command_node = nullptr;
	godot::InfoProxyNode *proxy_node = nullptr;
	bool use_producer = false;

	GameNodeTestContextWithProduction(bool with_producer = false) : use_producer(with_producer) {
		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");

		// Create basic gunbot prefab for testing
		auto prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		prefab->set_prefab_name("gunbot");
		prefab->set_hitpoint(100);
		prefab->set_armor(0);
		prefab->set_damage_x10(100);
		prefab->set_reload_x10(1);
		prefab->set_range_x10(300);
		prefab->set_windup_x10(1);
		prefab->set_production_duration(1);  // 1 second production time
		game_node->get_unit_prefabs().push_back(prefab);

		auto producer_prefab = Ref<godot::UnitPrefab>(memnew(godot::UnitPrefab));
		producer_prefab->set_prefab_name("armorbot");
		producer_prefab->set_producer(use_producer);
		game_node->get_unit_prefabs().push_back(producer_prefab);

		command_node = memnew(godot::CommandNode);
		command_node->set_ref_game_node(NodePath("/root/GameNode"));
		game_node->add_child(command_node);

		proxy_node = memnew(godot::InfoProxyNode);
		proxy_node->set_ref_game_node(NodePath("/root/GameNode"));
		proxy_node->set_refresh_tick(1);
		game_node->add_child(proxy_node);

		SceneTree::get_singleton()->get_root()->add_child(game_node);
		game_node->init_from_level(Dictionary());

		spawn_base_central_entity({50, 50});
	}

	void spawn_base_central_entity(octopus::Vector const &position) {
		game_node->get_world().ecs.entity()
			.is_a(game_node->get_world().ecs.prefab("armorbot"))
			.set<octopus::Team>({0})
			.set<octopus::PlayerAppartenance>({0})
			.set<octopus::Position>({position, {0, 0}});
	}

	~GameNodeTestContextWithProduction() {}
};

// Test 1: Verify queue_production works with production setup
void test_commandnode_production_with_producer() {
	GameNodeTestContextWithProduction context(true);

	context.game_node->tick();
	int const initial_count = count_entities_for_player(context.proxy_node, 0);

	context.command_node->queue_production(0, "gunbot");
	context.game_node->tick();

	context.game_node->tick(55);

	int const produced_unit_count = count_entities_for_player(context.proxy_node, 0);
	CHECK(produced_unit_count == initial_count + 1);
}

void test_commandnode_production_without_producer() {
	GameNodeTestContextWithProduction context(false);

	context.game_node->tick();
	int const initial_count = count_entities_for_player(context.proxy_node, 0);

	context.command_node->queue_production(0, "gunbot");
	context.game_node->tick();

	context.game_node->tick(55);

	int const produced_unit_count = count_entities_for_player(context.proxy_node, 0);
	CHECK(produced_unit_count == initial_count);
}

void test_commandnode_production_two_producers_parallel_output() {
	GameNodeTestContextWithProduction context(true);

	context.game_node->tick();
	context.spawn_base_central_entity({60, 50});
	context.game_node->tick();
	int const initial_count = count_entities_for_player(context.proxy_node, 0);

	context.command_node->queue_production(0, "gunbot");
	context.game_node->tick();
	context.command_node->queue_production(0, "gunbot");
	context.game_node->tick();
	context.game_node->tick(55);

	int const produced_unit_count = count_entities_for_player(context.proxy_node, 0);
	CHECK(produced_unit_count == initial_count + 2);
}
