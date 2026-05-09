#pragma once

#include <flecs.h>
#include <soci/soci.h>
#include <soci/connection-pool.h>
#include <memory>
#include <string>

namespace Database {

struct Connection {
    std::shared_ptr<soci::connection_pool> pool;
};

struct Snapshot {
    std::shared_ptr<std::vector<unsigned char>> buffer;
    std::string destination_filename;
};

struct InputFiles {
    std::vector<std::string> filepaths;
};

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}
