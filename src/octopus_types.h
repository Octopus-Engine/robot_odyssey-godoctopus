#pragma once

#include "octopus/commands/basic/ability/CastCommand.hh"
#include "octopus/commands/basic/move/AttackCommand.hh"
#include "octopus/commands/basic/move/MoveCommand.hh"
#include "octopus/commands/basic/rally_point/SetRallyPointCommand.hh"
#include "octopus/commands/queue/CommandQueue.hh"
#include "octopus/world/StepContext.hh"

typedef std::variant<octopus::NoOpCommand, octopus::MoveCommand, octopus::AttackCommand, octopus::CastCommand, octopus::SetRallyPointCommand
	> custom_variant;
typedef octopus::DefaultStepManager custom_step_manager;
typedef octopus::CommandQueue<custom_variant> custom_queue;

#define WORLD_SCALE 1.
#define TICK_RATE 50
