#pragma once

#include <flecs.h>
#include <unordered_map>

#include "../plugin/interface.hpp"

namespace plugin_example {

// Holds ECS world and GUID-to-entity mapping
class PluginState {
public:
    flecs::world ecs;
    std::unordered_map<Plugin::GUID, flecs::entity> guid_map;
    PluginState();
    ~PluginState();
    flecs::entity get_or_create_entity(Plugin::GUID guid);
};

extern PluginState g_state;

struct module {
    module(flecs::world& ecs);
};

} // namespace plugin_example