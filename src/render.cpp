#include "render.hpp"

#include <tracy/Tracy.hpp>


#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"
//#include "flecs_components_physics.h"
//#include "flecs_components_gui.h"
//#include "flecs_components_input.h"
//#include "flecs_systems_transform.h"
//#include "flecs_systems_physics.h"
//#include "flecs_systems_sokol.h"

#include "componentsPawn.hpp"
#include "componentsBuilding.hpp"
#include "ticks.hpp"

namespace Render{






module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();


    ecs.import<flecs::components::transform>();
    ecs.import<flecs::components::graphics>();
    ecs.import<flecs::components::geometry>();
//    ecs.import<flecs::components::gui>();
//    ecs.import<flecs::components::physics>();
//    ecs.import<flecs::components::input>();
//    ecs.import<flecs::systems::transform>();
//    ecs.import<flecs::systems::physics>();
//    ecs.import<flecs::systems::sokol>();

    ecs.import<Pawn::module>();
    ecs.import<Building::module>();
    ecs.import<Ticks::module>();







    // Add GUI components to granary
    // TODO: By using a PreFab, should be able to do this to all buildings of type
    // TODO: This should be an observer looking for when new pawns are added to the buildingsParent
    ecs.defer_begin();

    Building::buildingsParent.children([](flecs::entity building) {
        const Building::Location* loc = building.get<Building::Location>();
        const Building::BuildingUI* size = building.get<Building::BuildingUI>();
        building.set<flecs::components::transform::Position3>({(float) loc->x, 0.1, (float) loc->y});
        building.set<flecs::components::geometry::Box>({(float) size->sizeX, 2, (float) size->sizeY});
        building.set<flecs::components::graphics::Rgb>({20, 0, 0});
    });
    ecs.defer_end();






    // Add rendering components to Pawns
    //  As iterating, must defer
    // TODO: This should be an observer looking for when new pawns are added to the pawnsParent
    ecs.defer_begin();
    Pawn::pawnsParent.children([](flecs::entity pawn) {
            // Get the location
            flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
            const GridCellStatic* loc = currentCell.get<GridCellStatic>();
            // Then set renderable components
            pawn.set<flecs::components::transform::Position3>({(float) loc->x, 0.1, (float) loc->y});
            pawn.set<flecs::components::geometry::Box>({0.1, 0.8, 0.1});
            pawn.set<flecs::components::graphics::Rgb>({165, 42, 42});
        });
    ecs.defer_end();






    auto updatePawnRenderLocation_sys = ecs.system<Pawn::Position, flecs::components::transform::Position3>("Update Pawn Render Location")
    .tick_source(Ticks::tick_render)
    .with<Pawn::PawnOccupying>(flecs::Wildcard)
    .each([](flecs::entity pawn, Pawn::Position& p, flecs::components::transform::Position3& renderPos){
        // Get cell from pawn occupying
        flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
        const GridCellStatic* loc = currentCell.get<GridCellStatic>();
        renderPos.x = loc->x + p.x/2;
        renderPos.z = loc->y + p.y/2;
    });












    







};


}