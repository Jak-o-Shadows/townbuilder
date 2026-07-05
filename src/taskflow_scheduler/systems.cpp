#include <tracy/Tracy.hpp>

#include "taskflow_scheduler/module.hpp"

#include "msgLogging/module.hpp"

#include <iostream>
#include <memory>
#include <sstream>

namespace TaskflowScheduler {

std::shared_ptr<spdlog::logger> systemsLogger;

systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<TaskflowScheduler::components>();

    systemsLogger->trace("Other flecs modules imported");

    auto system_query =  ecs.query_builder()
        .with(flecs::System)
        .with(flecs::Phase).cascade(flecs::DependsOn)
        .without(flecs::Disabled).up(flecs::DependsOn)
        .without(flecs::Disabled).up(flecs::ChildOf)
        .build();

    // System to list all other systems
    ecs.system("SystemLister")
        .run([system_query](flecs::iter& it) {
            ZoneScopedN("SystemLister");
            systemsLogger->debug("Listing all systems:");
            std::ostringstream msg;
            msg << "Systems: ";
            system_query.each([&msg](flecs::entity e) {
                msg << e.path().c_str() << "-> ";
            });
            msg << "END FRAME";
            systemsLogger->debug("{}", msg.str());
        });

}

}