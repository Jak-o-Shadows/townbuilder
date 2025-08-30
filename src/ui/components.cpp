#include "ui/module.hpp"

#include "msgLogging/module.hpp"

namespace UI {

std::shared_ptr<spdlog::logger> logger;

components::components(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");

    // Register UI components so I can see them in the flecs explorer
    ecs.component<PawnJobs>()
        .member<int>("unemployed")
        .member<int>("woodcutter");
	logger->trace("Components Registered");
};

}