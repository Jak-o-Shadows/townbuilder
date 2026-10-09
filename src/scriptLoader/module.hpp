#pragma once

#include <flecs.h>

#include <string>
#include <vector>

namespace ScriptLoader {

struct ScriptsToLoad {
    std::vector<std::string> filepaths;
    bool loaded = false;
};

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}