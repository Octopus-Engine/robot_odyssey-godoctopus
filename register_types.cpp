#include <register_types.h>

// #include <godot_cpp/core/defs.hpp>
// #include <godot_cpp/core/class_db.hpp>
// #include <godot_cpp/godot.hpp>

#include "vat/VatLibrary.h"
#include "vat/VatMultiMeshInstance.h"
#include "vat/VatAnimationTrack.h"
#include "godoctopus/game/GameNode.h"

void initialize_godoctopus2_module(ModuleInitializationLevel p_level) {
  if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
	return;
  }

  // REGISTER CLASSES HERE LATER

  // VAT
	ClassDB::register_class<godot::VatLibrary>();
	ClassDB::register_class<godot::VatMultiMeshInstance>();
	ClassDB::register_class<godot::VatAnimationTrack>();
	// Godoctopus
  ClassDB::register_class<godot::GameNode>();
}

void uninitialize_godoctopus2_module(ModuleInitializationLevel p_level) {
  if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
	return;
  }
  // DO NOTHING
}
