#include "taskflow_scheduler/module.hpp"

#include "msgLogging/module.hpp"

#include <memory>


namespace TaskflowScheduler {

std::shared_ptr<spdlog::logger> systemsLogger;

systems::systems(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<TaskflowScheduler::components>();

    systemsLogger->trace("Other flecs modules imported");

}

}