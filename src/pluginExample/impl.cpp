// Include paths are different because the example is being compiled separately
#include "interface.hpp"
#include "impl.hpp"

#include <iostream>
#include <format>
#include <Eigen/Dense>


namespace plugin_example {


PluginState::PluginState() : ecs() {
    ecs.import<module>();
};
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

// Define flecs world & systems
module::module(flecs::world& ecs) {
    //std::cout << "PluginExample module initializing..." << std::endl;
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    //std::cout << std::format("PluginExample module entity created: {}", std::string(m.path())) << std::endl;

    // Register the Plugin types as components
//    ecs.component<Plugin::GUID>()
//        .member<Plugin::GUID>("id");
    ecs.component<Plugin::ComplexChannel>()
        //.member<std::complex<double>*>("data")
        .member<std::size_t>("count");
    ecs.component<Plugin::TickInput>()
        .member<double>("time");
        //.member<Plugin::ComplexChannel[4]>("channels");
    ecs.component<Plugin::PluginResults>()
        .member<bool>("success")
        .member<int>("max_location");
    //std::cout << "Registered Plugin components" << std::endl;

    ecs.system<const Plugin::TickInput>("PluginTick")
        .term_at(0).in()
        .each([](flecs::entity e, const Plugin::TickInput& input) {
            //std::cout << std::format("PluginTick System called for entity {}", std::string(e.path())) << std::endl;
            int max_loc[4];
            for (int ch_idx=0;ch_idx<4;ch_idx++){
                max_loc[ch_idx] = -1;  // Initialise to -1 to make sure we're not getting confused
                Eigen::Map<const Eigen::VectorXcd> channel(input.channels[ch_idx].data, input.channels[ch_idx].count);
                //std::cout << std::format("####### Channel {} ########", ch_idx) << std::endl;
                //std::cout << input.channels[ch_idx].data << std::endl;
                //std::cout << channel << std::endl;
                channel.cwiseAbs().maxCoeff(&max_loc[ch_idx]);  // cwiseAbs2 may be more efficient, but who cares
                //std::cout << std::format("Max Location is {}", max_loc[ch_idx]) << std::endl;
            }
            //std::cout << input.map.data[0] << std::endl;
            //std::cout << input.map.rows << ", " << input.map.cols << std::endl;
            std::complex<double> first_elem = input.map.data[0][0];
            //std::cout << std::format("first elem is: {} + {}j", first_elem.real(), first_elem.imag()) << std::endl;
            e.set<Plugin::PluginResults>({true, max_loc[0], max_loc[1], max_loc[2], max_loc[3], first_elem, input.map.rows, input.map.cols});
        });

    ecs.system("EveryFrame")
        .kind(flecs::OnUpdate)
        .run([](flecs::iter& it){
            //std::cout << "ECS World progressing..." << std::endl;
        });

    //std::cout << "PluginExample systems initialized." << std::endl;

};















// Define the exported functions for the DLL itnerface

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
    plugin_example::g_state.ecs.defer_begin();
    // Store the input
    e.set<Plugin::TickInput>(input);
    //std::cout << std::format("Setting input for entity {} with guid {}", std::string(e.path()), id) << std::endl;
    plugin_example::g_state.ecs.defer_end();

    plugin_example::g_state.ecs.progress();  // TODO: Put the delta time in
    //std::cout << "Progressed ECS"<< std::endl;

}

Plugin::PluginResults plugin_get_results(const Plugin::GUID id) {
    flecs::entity e = plugin_example::g_state.get_or_create_entity(id);
    //std::cout << std::format("Getting results for entity {} with guid {}", std::string(e.path()), id) << std::endl;
    const Plugin::PluginResults* results = e.get<Plugin::PluginResults>();
    if (results) {
        return *results;
    } else {
        return Plugin::PluginResults{false, -1};
    }
}

}


}