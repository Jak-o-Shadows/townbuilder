#include "impl.hpp"
#include "impl.hpp"

namespace plugin_example {


PluginState::PluginState() : ecs() {};
PluginState::~PluginState() {};

flecs::entity PluginState::get_or_create_entity(townbuilder::plugin::GUID guid) {
    std::unordered_map<townbuilder::plugin::GUID, flecs::entity>::iterator it = guid_map.find(guid);
    if (it != guid_map.end()) {
        return it->second;
    }
    flecs::entity e = ecs.entity();//.set<townbuilder::plugin::GUID>({guid});
    guid_map[guid] = e;
    return e;
}

PluginState g_state;

extern "C" {

void plugin_init() {
    // Initialise the ECS so the plugin can store entities
    g_state = PluginState();
}

void entity_init(const townbuilder::plugin::GUID id) {
    plugin_example::g_state.get_or_create_entity(id);
}

void plugin_close(const townbuilder::plugin::GUID id) {
    std::unordered_map<townbuilder::plugin::GUID, flecs::entity>& map = plugin_example::g_state.guid_map;
    std::unordered_map<townbuilder::plugin::GUID, flecs::entity>::iterator it = map.find(id);
    if (it != map.end()) {
        it->second.destruct();
        map.erase(it);
    }
}

void plugin_tick(const townbuilder::plugin::GUID id, const townbuilder::plugin::TickInput& input) {
    flecs::entity e = plugin_example::g_state.get_or_create_entity(id);
    // Mock: just store time in entity
    //e.set<double>(input.time);
}

townbuilder::plugin::PluginResults plugin_get_results(const townbuilder::plugin::GUID id) {
    // Mock: always return success
    return townbuilder::plugin::PluginResults{true};
}

}


}