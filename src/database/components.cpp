#include "database/module.hpp"
#include "msgLogging/module.hpp"

namespace Database {

std::shared_ptr<spdlog::logger> componentsLogger;

components::components(flecs::world& ecs) {
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Database components module created");

};

}
