#include <register_types.h>

// #include <godot_cpp/core/defs.hpp>
// #include <godot_cpp/core/class_db.hpp>
// #include <godot_cpp/godot.hpp>

#include "vat/SmartMMeshLibrary.h"
#include "vat/SmartMultiMeshInstance.h"
#include "vat/VatLibrary.h"
#include "vat/VatMultiMeshInstance.h"
#include "vat/VatAnimationTrack.h"
#include "godoctopus/action/ActionNode.h"
#include "godoctopus/command/CommandNode.h"
#include "godoctopus/custom_signal/CustomSignalNode.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"
#include "godoctopus/display/particule/ParticleOrchestrator.h"
#include "godoctopus/entity_group/EntityGroup.h"
#include "godoctopus/entity_group/SelectionGroup.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/game/levels/AttackMoveDemoNode.h"
#include "godoctopus/game/prefabs/UnitPrefab.h"
#include "godoctopus/health_bar/HealthBarNode.h"
#include "godoctopus/info/InfoNode.h"
#include "godoctopus/info/VisionNode.h"
#include "godoctopus/pickable/PickerNode.h"
#include "godoctopus/proxy/InfoProxyNode.h"
#include "godoctopus/proxy/PlayerProxyNode.h"
#include "godoctopus/proxy/InputStatusProxyNode.h"
#include "godoctopus/proximity/ProximityChecker.h"
#include "godoctopus/nwfc/NotWaveFunctionCollapseNode.h"
#include "godoctopus/resource_producer/ResourceNodeEventBus.h"
#include "godoctopus/event_bus/ProductionNodeEventBus.h"
#include "godoctopus/triangulation/TriangulationNode.h"
#include "godoctopus/triangulation/DelaunayTriangulationNode.h"

void initialize_godoctopus2_module(ModuleInitializationLevel p_level) {
	if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// REGISTER CLASSES HERE LATER

	// VAT
	ClassDB::register_class<godot::SmartMMeshLibrary>();
	ClassDB::register_class<godot::SmartMultiMeshInstance>();
	ClassDB::register_class<godot::VatLibrary>();
	ClassDB::register_class<godot::VatMultiMeshInstance>();
	ClassDB::register_class<godot::VatAnimationTrack>();
	// Godoctopus
	ClassDB::register_class<godot::EntityGroup>();
	ClassDB::register_class<godot::SelectionGroup>();
	ClassDB::register_class<godot::PickerNode>();
	ClassDB::register_class<godot::CustomSignalNode>();
	ClassDB::register_class<godot::ActionNode>();
	ClassDB::register_class<godot::CommandNode>();
	ClassDB::register_class<godot::ParticuleSmartResource>();
	ClassDB::register_class<godot::ParticuleTypeResource>();
	ClassDB::register_class<godot::ParticleOrchestrator>();
	ClassDB::register_class<godot::ParticuleSmartMMesh>();
	ClassDB::register_class<godot::LevelNode>();
	ClassDB::register_class<godot::GameNode>();
	ClassDB::register_class<godot::HealthBarNode>();
	ClassDB::register_class<godot::InfoNode>();
	ClassDB::register_class<godot::StatsInfo>();
	ClassDB::register_class<godot::AbilityCastableInfo>();
	ClassDB::register_class<godot::InfoTargetResource>();
	ClassDB::register_class<godot::InfoProductionQueueResource>();
	ClassDB::register_class<godot::InfoAvailableActionResource>();
	ClassDB::register_class<godot::InfoProxyResource>();
	ClassDB::register_class<godot::InfoProxyNode>();
	ClassDB::register_class<godot::PlayerResourceEntryResource>();
	ClassDB::register_class<godot::PlayerUpgradeEntryResource>();
	ClassDB::register_class<godot::PlayerLoadoutRuneEntryResource>();
	ClassDB::register_class<godot::PlayerLoadoutRuneSlotResource>();
	ClassDB::register_class<godot::PlayerLoadoutUnitEntryResource>();
	ClassDB::register_class<godot::PlayerResourceProxyResource>();
	ClassDB::register_class<godot::PlayerProxyNode>();
	ClassDB::register_class<godot::InputStatusResourceCost>();
	ClassDB::register_class<godot::InputStatusResource>();
	ClassDB::register_class<godot::InputStatusProxyNode>();
	ClassDB::register_class<godot::VisionNode>();
	ClassDB::register_class<godot::ParticleOcherstrated>();
	ClassDB::register_class<godot::UnitPrefab>();
	ClassDB::register_class<godot::ProximityChecker>();
	ClassDB::register_class<godot::ResourceNodeEventBus>();
	ClassDB::register_class<godot::ProductionNodeEventBus>();
	ClassDB::register_class<godot::TriangulationNode>();
	ClassDB::register_class<godot::DelaunayTriangulationNode>();
	// Demo Nodes
	ClassDB::register_class<godot::AttackMoveDemoNode>();

	ClassDB::register_class<godot::NotWaveFunctionCollapseNode>();
}

void uninitialize_godoctopus2_module(ModuleInitializationLevel p_level) {
	if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	// DO NOTHING
}
