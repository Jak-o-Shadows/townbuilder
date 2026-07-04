#pragma once
#include <taskflow/taskflow.hpp>
#include <flecs.h>
#include <unordered_map>
#include <vector>

namespace TaskflowScheduler {

class Scheduler {
    tf::Executor executor;
    tf::Taskflow taskflow;
};

} // namespace TaskflowScheduler
