#include "msgLogging/module.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>
#include <flecs.h>

#include <iostream>
#include <format>



namespace Logging{

std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, std::shared_ptr<spdlog::sinks::sink> sink) {
    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(std::string(module.path()), sink);
    logger->set_level(spdlog::level::trace);  // Set to trace in the knowledge that the observer will fire soon and change it
    spdlog::register_logger(logger);
    // Add it as the module logger component
    //  This allows things other than within the namespace to access it
    module.set<Logger>({logger});

    return logger;
}


components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<components>();

    ecs.component<spdlog::level::level_enum>()
        .constant("trace",      spdlog::level::trace)
        .constant("debug",      spdlog::level::debug)
        .constant("info",       spdlog::level::info)
        .constant("warn",       spdlog::level::warn)
        .constant("err",        spdlog::level::err)
        .constant("critical",   spdlog::level::critical)
        .constant("off",        spdlog::level::off);

    ecs.component<LoggerControls>()
        .member<spdlog::level::level_enum>("level");
};

}