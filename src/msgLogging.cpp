#include "msgLogging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/pattern_formatter.h>
#include <flecs.h>

#include <iostream>
#include <format>



namespace Logging{

class FlecsWorldTimeFormatter : public spdlog::custom_flag_formatter {
public:
    explicit FlecsWorldTimeFormatter(std::shared_ptr<flecs::world> ecs) : ecs_(ecs) {}

    void format(const spdlog::details::log_msg& msg, const std::tm& tm_time, spdlog::memory_buf_t& dest) override {
        // Print the flecs world time. This lets you match more against the game time
        double world_time = ecs_->get_info()->world_time_total;
        std::string world_time_formatted = std::format("{:.3f} ", world_time);
        dest.append(world_time_formatted.data(), world_time_formatted.data() + world_time_formatted.size());
    }

    std::unique_ptr<spdlog::custom_flag_formatter> clone() const override {
        return spdlog::details::make_unique<FlecsWorldTimeFormatter>(*this);
    }

private:
    std::shared_ptr<flecs::world> ecs_;
};


std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, std::shared_ptr<spdlog::sinks::sink> sink) {
    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(std::string(module.path()), sink);
    logger->set_level(spdlog::level::trace);  // Set to trace in the knowledge that the observer will fire soon and change it
    spdlog::register_logger(logger);
    // Add it as the module logger component
    //  This allows things other than within the namespace to access it
    module.set<Logger>({logger});

    return logger;
}


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();

    // Set up the logger sink for all loggers as a singleton
    std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.log", true);
    ecs.set<LoggerSink>({sink});

    // Register and use the custom formatter
    //  Note that the order and function calls to set the pattern & the formatter are very specific
    //  It will NOT work if you set the pattern on the sink instead
    //  Why? Who knows.
    std::shared_ptr<flecs::world> ecs_ptr = std::make_shared<flecs::world>(ecs);
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<FlecsWorldTimeFormatter>('j', ecs_ptr);
    formatter->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%j] [%^%l%$] [%n] %v");
    // As the loggers are defined after this, must apply the formatter to the sink
    sink->set_formatter(std::move(formatter));

    // Set up the observer to update the log level of the logger
    ecs.observer<LoggerControls>("UpdateLogLevel")
        .event(flecs::OnSet)
        .each([](flecs::entity e, LoggerControls& c){
            const Logger *lg = e.get<Logger>();
            lg->logger->set_level(c.level);
            lg->logger->trace("Changing log level for {} to {}", std::string(e.path()), spdlog::level::to_string_view(c.level));
        });

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