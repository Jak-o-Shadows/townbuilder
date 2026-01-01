#pragma once

#include <flecs.h>

namespace Example {

struct async {
    async(flecs::world& ecs);
};

struct statemachine {
    statemachine(flecs::world& ecs);
};

}