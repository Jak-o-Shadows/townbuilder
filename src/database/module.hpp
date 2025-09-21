#pragma once

#include <flecs.h>
#include <soci/soci.h>
#include <memory>
#include <string>

namespace Database {

struct Connection {
    std::unique_ptr<soci::session> sql;
};

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}
