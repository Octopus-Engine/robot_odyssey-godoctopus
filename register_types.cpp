#include <register_types.h>

// #include <godot_cpp/core/defs.hpp>
// #include <godot_cpp/core/class_db.hpp>
// #include <godot_cpp/godot.hpp>

#include "vat/SmartMMeshLibrary.h"
#include "vat/SmartMultiMeshInstance.h"
#include "vat/VatLibrary.h"
#include "vat/VatMultiMeshInstance.h"
#include "vat/VatAnimationTrack.h"
#include "godoctopus/command/CommandNode.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"
#include "godoctopus/game/GameNode.h"
#include "godoctopus/game/levels/AttackMoveDemoNode.h"
#include "godoctopus/pickable/Pickable.h"

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
	ClassDB::register_class<godot::PickerNode>();
	ClassDB::register_class<godot::CommandNode>();
  ClassDB::register_class<godot::ParticuleSmartMMesh>();
  ClassDB::register_class<godot::GameNode>();

  // Demo Nodes
  ClassDB::register_class<godot::AttackMoveDemoNode>();
}

void uninitialize_godoctopus2_module(ModuleInitializationLevel p_level) {
  if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
	return;
  }
  // DO NOTHING
}
