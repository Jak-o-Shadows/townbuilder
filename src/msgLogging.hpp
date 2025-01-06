#pragma once
#include <stdint.h>

#include <io.h>

#include <flecs.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>


template <typename T>
flecs::entity_t registerModule(flecs::world& ecs, std::string full_module_name, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    return ecs.entity(full_module_name.c_str())
        .set<T>({ecs, level, sink});
}


namespace Logging {

struct Logger {
    std::shared_ptr<spdlog::logger> logger;
    spdlog::level::level_enum level;
    std::string logger_name;

    Logger() = default;
    void init(const std::string& name, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink){
        logger_name = name;
        level = level;
        // Create a logger with a shared sink
        logger = std::make_shared<spdlog::logger>(logger_name, sink);
        logger->set_level(level);
        spdlog::register_logger(logger);
    }

};

Logger* init_module_logger(flecs::entity& module, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);

struct module {
    module(flecs::world& ecs);
};

struct examplemodule {
    examplemodule() = default;
    examplemodule(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);
};



}