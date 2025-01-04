#pragma once
#include <stdint.h>

#include <io.h>

#include <flecs.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

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

struct module {
    module(flecs::world& ecs);
};

struct testmodule {
    testmodule() = default;
    testmodule(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);
};



}