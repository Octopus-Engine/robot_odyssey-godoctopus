#include "SpawnUnitRune.h"

#include "godoctopus/components/rune_load/RuneLoad.h"
#include "godoctopus/trigger_module/events/SpawnUnitEvent.h"
#include "godoctopus/trigger_module/TriggerSystem.h"
#include "godoctopus/trigger_module/TriggerTypes.h"
#include "godoctopus/trigger_module/conditions/AlwaysCondition.h"

#include "godoctopus/trigger_module/BuffDeclarer.h"

void declare_spawn_unit_triggers(flecs::world &ecs, octopus::PositionContext const &ctx) {
	declare_trigger_buff<SpawnCloneUnitRune>(ecs);
	declare_trigger_system<SpawnCloneUnitRune, trigger_module::Death, AlwaysCondition, SpawnCopyEntityEvent<DefaultRune>>(ecs);
}
