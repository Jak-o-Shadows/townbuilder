#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include <io.h>

#include <flecs.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/details/synchronous_factory.h>

#include <tracy/Tracy.hpp>

#include <spdlog/formatter.h>

namespace Logging {

// Flecs Components
struct LoggerSink{
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;
    std::shared_ptr<spdlog::formatter> formatter;
};

struct LoggerControls {
    spdlog::level::level_enum level;
};

struct Logger {
    std::shared_ptr<spdlog::logger> logger;
};

/**
 * @brief A thread-safe spdlog sink that sends log messages to the Tracy profiler.
 *
 * @tparam Mutex The mutex type to use for thread safety. This is templated to match spdlog's design
 */
template<typename Mutex>
class tracy_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    tracy_sink() = default;

protected:
    /**
     * @brief The core sink method that processes the log message.
     * It formats the message and sends it to Tracy with a color corresponding to the log level.
     */
    void sink_it_(const spdlog::details::log_msg& msg) override {
        // Use the formatter provided by spdlog to format the message into a buffer.
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        // TracyMessageC requires a null-terminated string.
        formatted.push_back('\0');

        // Select a color for the message in Tracy based on the log level.
        uint32_t color = 0;
        switch (msg.level) {
            case spdlog::level::trace:    color = 0x808080; break; // Gray
            case spdlog::level::debug:    color = 0x00BFFF; break; // DeepSkyBlue
            case spdlog::level::info:     color = 0xFFFFFF; break; // White
            case spdlog::level::warn:     color = 0xFFD700; break; // Gold
            case spdlog::level::err:      color = 0xFF4500; break; // OrangeRed
            case spdlog::level::critical: color = 0xDC143C; break; // Crimson
            default: break;
        }

        // Send the formatted message to Tracy.
        TracyMessageC(formatted.data(), formatted.size() - 1, color);
    }

    /**
     * @brief The flush method. Tracy sends messages immediately, so this can be empty.
     */
    void flush_() override {}
};

using tracy_sink_mt = tracy_sink<std::mutex>;


std::shared_ptr<spdlog::logger> init_module_logger(flecs::entity& module, const std::vector<std::shared_ptr<spdlog::sinks::sink>>& sinks);

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

struct examplemodule {
    examplemodule(flecs::world& ecs);
};



}