#pragma once
#include <stdint.h>

#include <io.h>

#include <flecs.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Logging {

// Flecs Components
struct LoggerSink{
    std::shared_ptr<spdlog::sinks::sink> sink;
};

struct LoggerControls {
    spdlog::level::level_enum level;
};

struct Logger {
    std::shared_ptr<spdlog::logger> logger;
};



std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, std::shared_ptr<spdlog::sinks::sink> sink);

struct module {
    module(flecs::world& ecs);
};

struct examplemodule {
    examplemodule(flecs::world& ecs);
};



}