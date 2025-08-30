#pragma once

#include <flecs.h>

namespace UI{

struct components {
    components(flecs::world& ecs);
};

struct PawnJobs{
    int unemployed;
    int woodcutter;
};

}

