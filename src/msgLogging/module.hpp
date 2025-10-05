#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include <io.h>

#include <flecs.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Logging {

// Flecs Components
struct LoggerSink{
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;
};

struct LoggerControls {
    spdlog::level::level_enum level;
};

struct Logger {
    std::shared_ptr<spdlog::logger> logger;
};



std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, const std::vector<std::shared_ptr<spdlog::sinks::sink>>& sinks);

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

struct examplemodule {
    examplemodule(flecs::world& ecs);
};



}