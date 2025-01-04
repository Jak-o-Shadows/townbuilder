#include "msgLogging.hpp"

#include <iostream>

namespace Logging{

module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();

}

testmodule::testmodule(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
    flecs::entity m = ecs.module<testmodule>();

    m.set<Logging::Logger>({});
    Logging::Logger* lg = m.get_mut<Logging::Logger>();
    lg->init(std::string(m.path()), level, sink);
    lg->logger->trace("logger created");
}

}