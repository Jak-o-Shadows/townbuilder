#pragma once

#include <flecs.h>

namespace TaskflowScheduler {

struct TaskflowExempt {};

void build_taskflow_graph(flecs::world& ecs);

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}