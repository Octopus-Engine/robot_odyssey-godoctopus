#pragma once

#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"

#include "godoctopus/game/prefabs/UnitPrefab.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"

struct GameNodeTestContextWithCustomPrefab {
	godot::GameNode *game_node = nullptr;
	godot::ActionNode *action_node = nullptr;
	godot::InfoProxyNode *proxy_node = nullptr;

	GameNodeTestContextWithCustomPrefab(Ref<godot::UnitPrefab> custom_prefab_1, Ref<godot::UnitPrefab> custom_prefab_2 = Ref<godot::UnitPrefab>()) {
		game_node = memnew(godot::GameNode);
		game_node->set_name("GameNode");
		game_node->get_unit_prefabs().push_back(custom_prefab_1);
		if (custom_prefab_2.is_valid()) {
			game_node->get_unit_prefabs().push_back(custom_prefab_2);
		}

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

	~GameNodeTestContextWithCustomPrefab() {
		if (game_node && game_node->get_parent()) {
			game_node->get_parent()->remove_child(game_node);
		}
		if (game_node) {
			game_node->queue_free();
			game_node = nullptr;
		}
		action_node = nullptr;
		proxy_node = nullptr;
	}
};
