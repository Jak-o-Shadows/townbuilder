#pragma once

#include <flecs.h>

namespace Ticks{

struct module {
    module(flecs::world& ecs);
   
};

extern flecs::entity tick_100_Hz;
extern flecs::entity tick_pawn_behaviour;
extern flecs::entity tick_ui;
extern flecs::entity tick_render;


}