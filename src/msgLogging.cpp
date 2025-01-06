#include "msgLogging.hpp"

#include <iostream>




namespace Logging{

Logger* init_module_logger(flecs::entity& module, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    module.set<Logging::Logger>({});
    Logger* lg = module.get_mut<Logger>();
    lg->init(std::string(module.path()), level, sink);
    std::cout << module.path() << " .. ";
    lg->logger->trace("logger created");
    std::cout << " done " << std::endl;
    return lg;
}


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();

}

examplemodule::examplemodule(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    flecs::entity m = ecs.module<examplemodule>();
    Logging::Logger* lg = Logging::init_module_logger(m, level, sink);
    lg->logger->trace("Module Created");
}

}