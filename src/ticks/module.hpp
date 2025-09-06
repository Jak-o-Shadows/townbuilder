#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <flecs.h>

namespace Ticks{

struct module {
    module(flecs::world& ec);
   
};

extern flecs::entity tick_100_Hz;
extern flecs::entity tick_pawn_behaviour;
extern flecs::entity tick_plugin;
extern flecs::entity tick_python;
extern flecs::entity tick_ui;
extern flecs::entity tick_render;


}