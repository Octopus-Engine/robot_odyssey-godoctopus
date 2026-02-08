#pragma once

#include "godoctopus/game/GameNode.h"
#include "core/variant/dictionary.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"
#include "godoctopus/display/particule/ParticleOrchestrator.h"

namespace godot {

class AttackMoveDemoNode : public LevelNode {
	GDCLASS(AttackMoveDemoNode, LevelNode)

	SET_GET_NODE_PATH(ParticuleSmartMMesh, particules);
	SET_GET_NODE_PATH(ParticleOrchestrator, particule_orchestrator);
	SET_GET_NODE_PATH(VatLibrary, vat_library);
	SET_GET_PARAM_DEF(String, unit1, "earbot");
	SET_GET_PARAM_DEF(String, unit2, "tallbot");
	SET_GET_PARAM_DEF(int, count1, 10);
	SET_GET_PARAM_DEF(int, count2, 10);

public:
	void setup(Dictionary const &meta_data, GameNode &game) override;
	void system_setup(Dictionary const &meta_data, GameNode &game) override;

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		BIND_NODE_PATH(AttackMoveDemoNode, ParticuleSmartMMesh, particules);
		BIND_NODE_PATH(AttackMoveDemoNode, ParticleOrchestrator, particule_orchestrator);
		BIND_NODE_PATH(AttackMoveDemoNode, VatLibrary, vat_library);
		ADD_SIMPLE_PROP(AttackMoveDemoNode, STRING, unit1);
		ADD_SIMPLE_PROP(AttackMoveDemoNode, INT, count1);
		ADD_SIMPLE_PROP(AttackMoveDemoNode, STRING, unit2);
		ADD_SIMPLE_PROP(AttackMoveDemoNode, INT, count2);
	}

	void init_nodes() {
		INIT_NODE_PATH(ParticuleSmartMMesh, particules);
		INIT_NODE_PATH(ParticleOrchestrator, particule_orchestrator);
		INIT_NODE_PATH(VatLibrary, vat_library);
	}
};

}

void demo_setup(Dictionary const &meta_data, godot::GameNode &game);
