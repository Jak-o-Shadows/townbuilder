#include "msgLogging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <iostream>




namespace Logging{

std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, std::shared_ptr<spdlog::sinks::sink> sink) {
    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(std::string(module.path()), sink);
    logger->set_level(spdlog::level::trace);  // Set to trace in the knowledge that the observer will fire soon and change it
    spdlog::register_logger(logger);
    // Add it as the module logger component
    //  This allows things other than within the namespace to access it
    module.set<Logger>({logger});
    std::cout << "Set Logger for " << module.path() << std::endl;

    return logger;
}


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();

    // Set up the logger sink for all loggers as a singleton
    ecs.set<LoggerSink>({std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.log", true)});
    std::cout << "Logger Sink Created" << std::endl;

    ecs.observer<LoggerControls>("UpdateLogLevel")
        .event(flecs::OnSet)
        .each([](flecs::entity e, LoggerControls& c){
            const Logger *lg = e.get<Logger>();
            std::cout << "Got logger" << std::endl;
            lg->logger->set_level(c.level);
            std::cout << "Changing log level for " << e.path() << " to " << c.level << std::endl;
        });
    std::cout << "Logger Observer Created" << std::endl;

}

// Example of how to use it
std::shared_ptr<spdlog::logger> logger;

examplemodule::examplemodule(flecs::world& ecs) {
    flecs::entity m = ecs.module<examplemodule>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");
}

}