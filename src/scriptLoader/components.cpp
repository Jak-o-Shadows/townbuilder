#include "scriptLoader/module.hpp"

#include "msgLogging/module.hpp"

namespace ScriptLoader {

static std::shared_ptr<spdlog::logger> componentsLogger;

components::components(flecs::world& ecs) {
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Components module created");

    ecs.component<ScriptsToLoad>().add(flecs::Singleton);

    componentsLogger->trace("Components Registered");
}

}