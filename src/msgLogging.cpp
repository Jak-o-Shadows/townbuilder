#include "msgLogging.hpp"

#include <iostream>




namespace Logging{

std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    module.set<Logging::Logger>({});
    Logger* lg = module.get_mut<Logger>();
    lg->init(std::string(module.path()), level, sink);
    lg->logger->trace("logger created");
    return lg->logger;
}


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();

}

// Example of how to use it
std::shared_ptr<spdlog::logger> logger;

examplemodule::examplemodule(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    flecs::entity m = ecs.module<examplemodule>();
    logger = Logging::init_module_logger(m, level, sink);
    logger->trace("Module Created");
}

}