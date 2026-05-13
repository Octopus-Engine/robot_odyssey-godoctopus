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

struct PrefabProductionDeclarer {
	flecs::world &ecs;
	const char *producer_name;

	template<typename BotType>
	void operator()() const {
		ecs.prefab(producer_name).add<octopus::ProductionQueue>(ecs.component(BotType::naming()));
	}
};

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

		command_node = memnew(godot::CommandNode);
		command_node->set_ref_game_node(NodePath("/root/GameNode"));
		game_node->add_child(command_node);

		proxy_node = memnew(godot::InfoProxyNode);
		proxy_node->set_ref_game_node(NodePath("/root/GameNode"));
		proxy_node->set_refresh_tick(1);
		game_node->add_child(proxy_node);

		SceneTree::get_singleton()->get_root()->add_child(game_node);
		game_node->init_from_level(Dictionary());

		// Setup producer if requested
		if (use_producer) {
			setup_producer();
		}

		// Create a base_central entity for player 0 regardless of producer setup, so we can verify production results
		game_node->get_world().ecs.entity()
			.is_a(game_node->get_world().ecs.prefab("base_central"))
			.set<octopus::Team>({0})
			.set<octopus::PlayerAppartenance>({0})
			.set<octopus::Position>({{50, 50}, {0, 0}});
	}

	void setup_producer() {
		flecs::world &ecs = game_node->get_world().ecs;

		// Declare base_central prefab with ProductionQueue
		ecs.prefab("base_central")
			.add<octopus::ProductionQueue>();

		// Add all bot types as available productions for base_central
		for_each_bot_type(PrefabProductionDeclarer{ecs, "base_central"});
	}

	~GameNodeTestContextWithProduction() {}
};

// Test 1: Verify queue_production works with production setup
void test_commandnode_production_with_producer() {
	// 1. Setup test context with production setup (includes base_central producer)
	GameNodeTestContextWithProduction context(true);

	// 2. Initial tick to ensure setup is complete
	context.game_node->tick();

	// 3. Call queue_production to produce a gunbot for player 0
	context.command_node->queue_production(0, "gunbot");
	context.game_node->tick();

	// 4. Wait for production to complete (~50 ticks for 1 second at 50Hz)
	// Production time should be defined in the gunbot prefab declaration
	context.game_node->tick(55);

	// 5. Verify that a gunbot unit was produced
	// We check this by examining the world state through the proxy node
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;

		// Count units for player 0 that are gumbots (or newly produced units)
		int produced_unit_count = 0;
		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_player() == 0) {
				produced_unit_count++;
			}
		}

		// Should have at least one unit produced by now
		CHECK(produced_unit_count > 1);
	}
}

// Test 2: Verify queue_production doesn't produce units without producer
void test_commandnode_production_without_producer() {
	// 1. Setup test context without AttackMoveDemoNode (no base_central producer)
	GameNodeTestContextWithProduction context(false);

	// 2. Initial tick
	context.game_node->tick();

	// 3. Attempt to call queue_production (should have no effect without producer)
	context.command_node->queue_production(0, "gunbot");
	context.game_node->tick();

	// 4. Wait for what would be production time
	context.game_node->tick(55);

	// 5. Verify that no units were produced
	{
		auto locker = context.proxy_node->get_data_locker();
		auto const &proxy_map = locker.proxy_map;

		// No units should be produced for player 0 without a producer
		int produced_unit_count = 0;
		for (auto const &[entity_id, proxy_data] : proxy_map) {
			if (proxy_data.get_player() == 0) {
				produced_unit_count++;
			}
		}


		CHECK(produced_unit_count == 1);
	}
}
