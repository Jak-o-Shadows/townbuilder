#include "buildings/module.hpp"

#include "msgLogging/module.hpp"

#include <iostream>


namespace Buildings{

// Handle extern entities
flecs::entity buildingsParent;
std::shared_ptr<spdlog::logger> logger;


components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");
    
    buildingsParent = ecs.entity("buildings");


    // Register components with reflection data
    
    ecs.component<Location>()
        .member<int>("x")
        .member<int>("y");
    ecs.component<Resources>()
        .member<int>("fish")
        .member<int>("stone")
        .member<int>("wood");
    ecs.component<BuildingUI>()
        .member<int>("sizeX")
        .member<int>("sizeY")
        .member<int>("doorX")
        .member<int>("doorY");
    logger->trace("Components Registered");
    

    flecs::entity building_prefab = ecs.prefab("building_prefab");







    // Start some buildings!

    flecs::entity granary_prefab = ecs.prefab<Granary_Prefab>()
        .is_a(building_prefab)
        .child_of(buildingsParent)
        .set<BuildingUI>({3, 3, -1, 0})
        .set<Resources>({0, 0, 0});
    logger->trace("Granary Prefab Created");








    logger->trace("Module Setup Complete");
};


}
