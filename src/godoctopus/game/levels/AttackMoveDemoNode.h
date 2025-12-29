#pragma once

#include "godoctopus/game/GameNode.h"
#include "core/variant/dictionary.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"

namespace godot {

class AttackMoveDemoNode : public LevelNode {
	GDCLASS(AttackMoveDemoNode, LevelNode)

	SET_GET_NODE_PATH(ParticuleSmartMMesh, particules);

public:
	void setup(Dictionary const &meta_data, GameNode &game) override;
	void system_setup(Dictionary const &meta_data, GameNode &game) override;

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		BIND_NODE_PATH(AttackMoveDemoNode, ParticuleSmartMMesh, particules);
	}

	void init_nodes() {
		INIT_NODE_PATH(ParticuleSmartMMesh, particules);
	}
};

}

void demo_setup(Dictionary const &meta_data, godot::GameNode &game);
