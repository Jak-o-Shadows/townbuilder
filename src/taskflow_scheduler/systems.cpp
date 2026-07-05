#include <tracy/Tracy.hpp>

#include "taskflow_scheduler/module.hpp"

#include "msgLogging/module.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <format>

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

    ecs.system("SystemInOutLister")
        .run([system_query, &ecs](flecs::iter& it) {
            ZoneScopedN("SystemInOutLister");
            systemsLogger->debug("Listing all systems in & outs:");

            system_query.each([&ecs](flecs::entity system_entity) {
                ecs_entity_t id = system_entity.id();
                const ecs_system_t *s = ecs_system_get(ecs.c_ptr(), id);
                if (!s || !s->query) {
                    systemsLogger->debug("{} : <no system data>", system_entity.path().c_str());
                    return;
                }
                std::ostringstream msg;
                msg << std::format("System: {} : ", system_entity.path().c_str());
                for (int t = 0; t < s->query->term_count; ++t) {
                    const ecs_term_t *term = &s->query->terms[t];
                    // term->id is the component id; term->inout is the access kind
                    const char *comp = ecs_get_name(ecs.c_ptr(), term->id);
                    const char *access = "unknown";
                    switch (term->inout) {
                        case EcsIn: access = "in"; break;
                        case EcsOut: access = "out"; break;
                        case EcsInOut: access = "inout"; break;
                        default: access = "other"; break;
                    }
                    msg << std::format("{}({}) ;", (comp ? comp : "<comp>"), access);
                }
                systemsLogger->debug("{} END", msg.str());
            });
        });

}

}