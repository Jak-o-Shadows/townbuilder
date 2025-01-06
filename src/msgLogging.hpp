#pragma once
#include <stdint.h>

#include <io.h>

#include <flecs.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>


template <typename T>
void registerModule(flecs::world& ecs, std::string full_module_name, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    ecs.entity(full_module_name.c_str())
        .set<T>({ecs, level, sink});
}


namespace Logging {

struct Logger {
    std::shared_ptr<spdlog::logger> logger;
    spdlog::level::level_enum level;

    Logger() = default;
    void init(const std::string& name, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink){
        level = level;
        // Create a logger with a shared sink
        logger = std::make_shared<spdlog::logger>(name, sink);
        logger->set_level(level);
        spdlog::register_logger(logger);
    }

};

std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);

struct module {
    module(flecs::world& ecs);
};

struct examplemodule {
    examplemodule() = default;
    examplemodule(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);
};



}