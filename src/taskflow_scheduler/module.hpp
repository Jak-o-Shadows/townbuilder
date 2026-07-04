#pragma once

#include <flecs.h>

namespace TaskflowScheduler {

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}