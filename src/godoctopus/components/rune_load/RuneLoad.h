#pragma once

#include "flecs.h"

struct DefaultRune {};

template<typename type>
struct RuneLoad
{
	int32_t qty = 0;
};

template<typename type>
void declare_rune_load_component(flecs::world &ecs)
{
	ecs.component<RuneLoad<type>>()
        .member("qty", &RuneLoad<type>::qty)
	;
}
