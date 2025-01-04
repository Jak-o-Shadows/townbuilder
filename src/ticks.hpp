#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <flecs.h>

namespace Ticks{

struct module {
    module() = default;
    module(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);
   
};

extern flecs::entity tick_100_Hz;
extern flecs::entity tick_pawn_behaviour;
extern flecs::entity tick_ui;
extern flecs::entity tick_render;


}