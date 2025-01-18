#include "render.hpp"

#include <tracy/Tracy.hpp>
#include <spdlog/spdlog.h>

#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"
#include "flecs_components_physics.h"
#include "flecs_components_gui.h"
#include "flecs_components_input.h"
#include "flecs_systems_transform.h"
#include "flecs_systems_physics.h"



#include "componentsPawn.hpp"
#include "componentsBuilding.hpp"
#include "ticks.hpp"

namespace Render{

std::shared_ptr<spdlog::logger> logger;


struct Game {
    flecs::entity window;
    flecs::entity level;
    
    flecs::components::transform::Position3 center;
    float size;        
};


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});  // TODO: Replace this with flecs script
    logger->trace("Module Created");
    std::cout << "Render Module Created" << std::endl;


    //logger->trace("Modules Imported");
    //std::cout << "Modules Imported" << std::endl;


    /*
    // Initialise game
    const float TileSize = 3.0;
    const float TileHeight = 0.5;
    const float PathHeight = 0.1;
    const float TileSpacing = 0.00;
    Game& g = ecs.ensure<Game>();
    g.center = {0, 0, 0};//{ to_x(map_width / 2), 0, to_z(map_height / 2) };
    // Get the map entity back out for working with for the moment
    /*
    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();
    std::cout << "Map Object gotten" << std::endl;
    int map_width = map->m_width;
    int map_height = map->m_height;
    g.size = map_width * (TileSize + TileSpacing) + 2;
    std::cout << "map GUI setup" << std::endl;
    */
   /*
   logger->trace("Map Gui Setup");
   std::cout << "Map Gui Setup" << std::endl;

    
    // Cannot figure out how to move these to render - so stuff it
    // Init UI
    
    flecs::components::graphics::Camera camera_data = {};
    camera_data.set_up(0, 1, 0);
    camera_data.set_fov(20);
    camera_data.near_ = 1.0;
    camera_data.far_ = 100.0;
    //auto camera = ecs.entity("Camera")
    auto camera = flecs::entity(ecs, "Camera");
        camera.set<flecs::components::transform::Position3>({0, 8.0, -9.0});
        camera.set<flecs::components::transform::Rotation3>({-0.5});
        camera.set<flecs::components::graphics::Camera>(camera_data);
        camera.add(flecs::game::CameraController);
    logger->trace("Camera Created");
    std::cout << "Camera Created" << std::endl;
    

    flecs::components::graphics::DirectionalLight light_data = {};
    light_data.set_direction(0.3, -1.0, 0.5);
    light_data.set_color(0.98, 0.95, 0.8);
    light_data.intensity = 0.01;
    //auto light = ecs.entity("Sun")
    auto light = flecs::entity(ecs, "Sun")
        .set<flecs::components::graphics::DirectionalLight>(light_data);
    logger->trace("Light Created");
    std::cout << "Light Created" << std::endl;

    
    flecs::components::gui::Canvas canvas_data = {};
    canvas_data.width = 800;
    canvas_data.height = 600;
    canvas_data.title = (char*)"TownBuilder";
    canvas_data.camera = camera.id();
    canvas_data.directional_light = light.id();
    canvas_data.ambient_light = {0.006, 0.005, 0.018};
    canvas_data.background_color = {0.15, 0.4, 0.6};
    canvas_data.fog_density = 1.0;
    //ecs.entity()
    flecs::entity(ecs, "Canvas_asdf")
        .set<flecs::components::gui::Canvas>(canvas_data);
    logger->trace("Canvas Created");
    std::cout << "Canvas Created" << std::endl;
    












    // Add GUI components to granary
    // TODO: By using a PreFab, should be able to do this to all buildings of type
    /*
    ecs.observer<flecs::ChildOf>("Observer_BuildingCreate")
        .term_at(0).second(Building::buildingsParent)
        .event(flecs::OnAdd)
        .each([](flecs::entity building){
            ZoneScopedN("Observer_BuildingCreate");
            const Building::Location* loc = building.get<Building::Location>();
            const Building::BuildingUI* size = building.get<Building::BuildingUI>();
            building.set<flecs::components::transform::Position3>({(float) loc->x, 0.1, (float) loc->y});
            building.set<flecs::components::geometry::Box>({(float) size->sizeX, 2, (float) size->sizeY});
            building.set<flecs::components::graphics::Color>({20, 0, 0});
        });
    */

   /*
    ecs.observer("Observer_PawnCreate")
        .with(flecs::ChildOf, Pawn::pawnsParent)
        .event(flecs::OnAdd)
        .each([](flecs::entity pawn){
            ZoneScopedN("Observer_PawnCreate");
            std::cout << "Pawn Creation Observer " << pawn.name() << std::endl;
            // Get the location
            flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
            const GridCellStatic* loc = currentCell.get<GridCellStatic>();
            // Then set renderable components
            pawn.set<flecs::components::transform::Position3>({(float) loc->x, 0.1, (float) loc->y});
            pawn.set<flecs::components::geometry::Box>({0.1, 0.8, 0.1});
            pawn.set<flecs::components::graphics::Color>({165, 42, 42});          
        });
    */

    /*
    // Add rendering components to Pawns
    //  As iterating, must defer
    // TODO: This should be an observer looking for when new pawns are added to the pawnsParent
    ecs.defer_begin();
    Pawn::pawnsParent.children([](flecs::entity pawn) {

        });
    ecs.defer_end();
    */



    /*
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
    */











    







};


}