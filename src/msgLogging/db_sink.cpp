#include "msgLogging/db_sink.hpp"
#include <spdlog/pattern_formatter.h>
#include <spdlog/common.h>

#include <iostream>

namespace Logging {

db_sink::db_sink(std::shared_ptr<soci::connection_pool> pool) : pool_(std::move(pool)) {
    // 
    try {
        std::cout << "Setting up database table for msg logging" << std::endl;
        std::cout << "Pool pointer is " << pool_.get() << std::endl;
        if (!pool_) {
            std::cout << "Cannot create 'logs' table because database connection pool is not available." << std::endl;
            return;
        }
        soci::session sql(*pool_);
        sql << "CREATE TABLE IF NOT EXISTS logs ("
                "time REAL, "
                "logger_name TEXT, "
                "level TEXT, "
                "message TEXT)";
        std::cout << "Table 'logs' created or already exists." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error creating table 'logs': " << e.what() << std::endl;
    }
}

void db_sink::sink_it_(const spdlog::details::log_msg& msg) {
    if (!pool_) {
        return;
    }

    try {
        soci::session sql(*pool_);

        // Format the message
        spdlog::memory_buf_t formatted;
        base_sink<std::mutex>::formatter_->format(msg, formatted);
        std::string message(formatted.data(), formatted.size());

        // Get log level as string
        std::string level = std::string(spdlog::level::to_string_view(msg.level));
        
        // Get logger name
        std::string logger_name = std::string(msg.logger_name);

        // Get time
        double time = std::chrono::duration<double>(msg.time.time_since_epoch()).count();

        sql << "INSERT INTO logs (time, logger_name, level, message) VALUES (:time, :logger_name, :level, :message)",
            soci::use(time), soci::use(logger_name), soci::use(level), soci::use(message);
    } catch (const std::exception& e) {
        // What to do here? Can't log the error.
        // Maybe write to stderr.
        fprintf(stderr, "Error in db_sink: %s\n", e.what());
    }
}

void db_sink::flush_() {
    // For now, this is a no-op. The sink writes immediately.
    // If we were batching writes, we would flush them here.
}

} // namespace Logging
