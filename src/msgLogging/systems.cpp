#include "msgLogging/module.hpp"
#include "msgLogging/db_sink.hpp"
#include "database/module.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>

#include <iostream>

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


systems::systems(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<systems>();

    // Set up the logger sink for all loggers as a singleton
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.log", true));
    sinks.push_back(std::make_shared<Logging::tracy_sink_mt>());
    ecs.set<LoggerSink>({sinks});

    // Register and use the custom formatter
    //  Note that the order and function calls to set the pattern & the formatter are very specific
    //  It will NOT work if you set the pattern on the sink instead
    //  Why? Who knows.
    std::shared_ptr<flecs::world> ecs_ptr = std::make_shared<flecs::world>(ecs);
    auto formatter = std::make_shared<spdlog::pattern_formatter>();
    formatter->add_flag<FlecsWorldTimeFormatter>('j', ecs_ptr);
    formatter->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%j] [%^%l%$] [%n] %v");

    ecs.get_mut<LoggerSink>().formatter = formatter;

    // As the loggers are defined after this, must apply the formatter to each sink
    for(auto& s : sinks)
        s->set_formatter(formatter->clone());

    // Set up the observer to update the log level of the logger
    ecs.observer<LoggerControls>("UpdateLogLevel")
        .event(flecs::OnSet)
        .each([](flecs::entity e, LoggerControls& c){
            const Logger& lg = e.get<Logger>();
            lg.logger->set_level(c.level);
            lg.logger->trace("Changing log level for {} to {}", std::string(e.path()), spdlog::level::to_string_view(c.level));
        });

    // When the database connection is ready, create and add the db_sink
    
    ecs.observer<Database::Connection>("Observer_CreateDatabaseLogSink")
        .event(flecs::OnSet)
        .each([ecs](Database::Connection& conn) {
            if (!conn.pool) {
                // Pool not created successfully, don't add sink.
                std::cout << "Database connection pool not available, cannot create database log sink." << std::endl;
                return;
            }

            // Get the logger sink singleton
            LoggerSink& sink_singleton = ecs.get_mut<LoggerSink>();

            // Create the db sink
            std::shared_ptr<Logging::db_sink> db_sink_instance = std::make_shared<Logging::db_sink>(conn.pool);
            
            // Set the formatter on the new sink
            if (sink_singleton.formatter) {
                db_sink_instance->set_formatter(sink_singleton.formatter->clone());
            }
            
            // Add the new sink to the central list
            sink_singleton.sinks.push_back(db_sink_instance);

            // Add the new sink to all existing loggers
            ecs.each<Logging::Logger>([&](Logging::Logger& logger) {
                logger.logger->trace("Adding database sink to logger");
                logger.logger->sinks().push_back(db_sink_instance);
                logger.logger->debug("Database sink added successfully");
            });
        });
    

}

// Example of how to use it
std::shared_ptr<spdlog::logger> logger;

examplemodule::examplemodule(flecs::world& ecs) {
    flecs::entity m = ecs.module<examplemodule>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::err});
    logger->trace("Module Created");
}

}