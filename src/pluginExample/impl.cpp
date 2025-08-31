// Include paths are different because the example is being compiled separately
#include "interface.hpp"
#include "impl.hpp"


namespace plugin_example {


PluginState::PluginState() : ecs() {};
PluginState::~PluginState() {};

flecs::entity PluginState::get_or_create_entity(Plugin::GUID guid) {
    std::unordered_map<Plugin::GUID, flecs::entity>::iterator it = guid_map.find(guid);
    if (it != guid_map.end()) {
        return it->second;
    }
    flecs::entity e = ecs.entity();//.set<townbuilder::Plugin::GUID>({guid});
    guid_map[guid] = e;
    return e;
}

PluginState g_state;

extern "C" {

void plugin_init() {
    // Initialise the ECS so the plugin can store entities
    g_state = PluginState();
}

void entity_init(const Plugin::GUID id) {
    plugin_example::g_state.get_or_create_entity(id);
}

void plugin_close(const Plugin::GUID id) {
    std::unordered_map<Plugin::GUID, flecs::entity>& map = plugin_example::g_state.guid_map;
    std::unordered_map<Plugin::GUID, flecs::entity>::iterator it = map.find(id);
    if (it != map.end()) {
        it->second.destruct();
        map.erase(it);
    }
}

void plugin_tick(const Plugin::GUID id, const Plugin::TickInput& input) {
    flecs::entity e = plugin_example::g_state.get_or_create_entity(id);
    // Store the input

    plugin_example::g_state.ecs.progress();  // TODO: Put the delta time in
}

Plugin::PluginResults plugin_get_results(const Plugin::GUID id) {
    // Mock: always return success
    return Plugin::PluginResults{true};
}

}


}