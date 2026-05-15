#pragma once

#include "flecs.h"

struct ProductionDone {
	flecs::entity entity;
	std::string production_name;
	int player;
};

#define PRODUCTION_NODE_EVENT_BUS "ProductionNodeEventBus"
