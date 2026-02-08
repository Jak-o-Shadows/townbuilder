#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/log_msg.h>

#include <soci/soci.h>
#include <soci/connection-pool.h>

#include <mutex>

namespace Logging {

class db_sink : public spdlog::sinks::base_sink<std::mutex> {
public:
    explicit db_sink(std::shared_ptr<soci::connection_pool> pool);

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

private:
    std::shared_ptr<soci::connection_pool> pool_;
};

}
