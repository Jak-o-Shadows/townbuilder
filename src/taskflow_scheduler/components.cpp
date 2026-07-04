#include "taskflow_scheduler/module.hpp"

#include "msgLogging/module.hpp"

#include <memory>

namespace TaskflowScheduler {

// Handle extern entities
std::shared_ptr<spdlog::logger> componentsLogger;


components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Module Created");

}

}