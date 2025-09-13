#pragma once

#include "plugin/interface.hpp"

#include <flecs.h>

namespace Plugin {

struct PluginGuid {
    GUID id;
};


struct components {
     components(flecs::world& ecs);
 };

struct systems {
     systems(flecs::world& ecs);
 };
 
 } // namespace Plugin
