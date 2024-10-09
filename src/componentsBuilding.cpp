#include "componentsBuilding.hpp"

namespace Building{

// Handle extern entities
flecs::entity buildingsParent;


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();
    
    buildingsParent = ecs.entity("buildings");


    // Register components with reflection data
    ecs.component<Building::Location>()
        .member<int>("x")
        .member<int>("y");
    ecs.component<Building::Resources>()
        .member<int>("fish")
        .member<int>("stone")
        .member<int>("wood");
    ecs.component<Building::BuildingUI>()
        .member<int>("sizeX")
        .member<int>("sizeY")
        .member<int>("doorX")
        .member<int>("doorY");


    // Start some buildings!
    auto granary = ecs.entity("Granary")
        .child_of(buildingsParent)
        .set<Location>({3, 8})
        .set<BuildingUI>({3, 3, -1, 0})
        .set<Resources>({0, 0, 0})
        .add<BuildingType>()
        ;
//        .set<flecs::components::transform::Position3>({3, 0.1, 8})
//        .set<flecs::components::geometry::Box>({2, 2, 2})
//        .set<flecs::components::graphics::Rgb>({20, 0, 0});





};


}
