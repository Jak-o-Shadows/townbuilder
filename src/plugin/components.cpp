#include "plugin/module.hpp"

#include "msgLogging/module.hpp"

#include <iostream>

namespace Plugin {
 
std::shared_ptr<spdlog::logger> componentsLogger;

components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    std::cout << "[DEBUG] Plugin::components constructor called" << std::endl;
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Module Created");

    //Register components
    std::cout << "[DEBUG] Registering PluginGuid component" << std::endl;
    ecs.component<PluginGuid>()
        .member<uint64_t>("id");
    std::cout << "[DEBUG] PluginGuid component registered with member<uint64_t>(\"id\")" << std::endl;
    componentsLogger->trace("Components Registered");
};
 
 } // namespace Plugin
