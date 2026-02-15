#pragma once

#include "plugin/interface.hpp"

#include <flecs.h>

namespace Plugin {

struct PluginGuid {
    uint64_t id;
};


struct components {
     components(flecs::world& ecs);
 };

struct systems {
     systems(flecs::world& ecs);
 };
 
 } // namespace Plugin
