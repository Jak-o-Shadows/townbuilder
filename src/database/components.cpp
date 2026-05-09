#include "database/module.hpp"
#include "msgLogging/module.hpp"

namespace Database {

std::shared_ptr<spdlog::logger> componentsLogger;

components::components(flecs::world& ecs) {
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Database components module created");

    ecs.component<Connection>()
        .add(flecs::Singleton);
    ecs.component<Snapshot>()
        .member<std::string>("destination_filename")
        .add(flecs::Singleton);
    ecs.component<InputFiles>()
        .member<std::vector<std::string>>("filepaths");
//        .add(flecs::Singleton);  // This causes a crash? No idea why.
    componentsLogger->trace("Components Registered");

};

}
