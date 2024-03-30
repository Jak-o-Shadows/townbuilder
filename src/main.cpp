//#define TRACY_ON_DEMAND

#include "gridMap.hpp"

#include "componentsPawn.hpp"
#include "componentsMap.hpp"
#include "componentsBuilding.hpp"
#include "componentsUi.hpp"
#include "logicPawn2.hpp"
#include "ticks.hpp"
//#include "render.hpp"

#include "tracy_zones.hpp"

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"
#include "flecs_components_physics.h"
#include "flecs_components_gui.h"
#include "flecs_components_input.h"
#include "flecs_systems_transform.h"
#include "flecs_systems_physics.h"
#include "flecs_systems_sokol.h"
#include "flecs_game.h"


// Tracy memory tracking
void* operator new(std::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}


#include <iostream>
#include <vector>
#include <random>


#define HFSM2_ENABLE_STRUCTURE_REPORT
#include <hfsm2/machine.hpp>


struct Game {
    flecs::entity window;
    flecs::entity level;
    
    flecs::components::transform::Position3 center;
    float size;        
};



////////////////////////////////////////////////////////////////////////////////


int main(int, char *[]) {

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::monitor>(); // Enable statistics in explorer

    ecs.import<flecs::components::transform>();
    ecs.import<flecs::components::graphics>();
    ecs.import<flecs::components::geometry>();
    ecs.import<flecs::components::gui>();
    ecs.import<flecs::components::physics>();
    ecs.import<flecs::components::input>();
    ecs.import<flecs::systems::transform>();
    ecs.import<flecs::systems::physics>();
    ecs.import<flecs::game>();
    ecs.import<flecs::systems::sokol>();


    // Define the map
    //  This is defined early because it isn't properly in the ECS, so initialisation order matters mroe
    // Have a base entity - lets the map class be accssible from the ECS, and is a parent,
    //   making it show nicer in the explorer
    flecs::entity mapEntity = ecs.entity("map");
    //  Each cell of the map is an entity
    const int map_width = 20;
    const int map_height = 20;
    // Stored in a vector for each access
    std::shared_ptr<grid> map = std::make_shared<grid>(grid(map_width, map_height, &ecs, mapEntity));
    // Add the map pointer to the map entity
    mapEntity.set<MapContainer>({map});
    //  Initially, fully connected
    for (int x = 0; x<map_width; x++){
        for (int y = 0; y<map_height; y++){
            flecs::entity thisCell = flecs::entity(ecs, map->get(x,y));
            if (x > 0){
                // Left valid
                thisCell.set<GridConnected>(map->get(x-1, y), {1});
            }
            if (x <map_width-1){
                // Right valid
                thisCell.set<GridConnected>(map->get(x+1, y), {2});
            }
            if (y > 0) {
                // Top valid
                thisCell.set<GridConnected>(map->get(x, y-1), {6});
            }
            if (y < map_height-1){
                // Bottom valid
                thisCell.set<GridConnected>(map->get(x, y+1), {3});
            }
        }
    }







    ecs.import<LogicPawn::module>();
    ecs.import<Pawn::module>();
    ecs.import<Ticks::module>();







    // Need to give the entities a parent so they show nicer in the flecs explorer
    flecs::entity resourcesParent = ecs.entity("resources");
    flecs::entity buildingsParent = ecs.entity("buildings");




    // Register components with reflection data
    ecs.component<Resources>()
        .member<int>("fish")
        .member<int>("stone")
        .member<int>("wood");
    
    // Register UI components so I can see them
    ecs.component<UiPawnJobs>()
        .member<int>("unemployed")
        .member<int>("woodcutter");




    






    // Map random-generation is VERY VERY primitive right now
    std::mt19937 rngMap;
    rngMap.seed(641331);
    std::bernoulli_distribution treeDist(0.2);

    // Define trees
    for (int x = 0; x<map_width; x++){
        for (int y = 0; y<map_height; y++){
            // Rectangular grid is simply connected if no trees
                // TODO: This currently lets you go onto tree-cells, but not out. Is that smart?
            flecs::entity thisCell = flecs::entity(ecs, map->get(x,y));
            if (treeDist(rngMap)){
                auto tree = ecs.entity()
                    .child_of(resourcesParent)
                    .set<Location>({x, y})
                    .set<Resources>({0, 100, 0})
                    .add<Nature>()
                    ;//
                    //.set<flecs::components::transform::Position3>({(float) x, 1, (float) y})
                    //.set<flecs::components::graphics::Rgb>({0, 255, 0})
                    //.set<flecs::components::geometry::Box>({0.1, 0.5, 0.1});
            }
        }
    }







    






    






    






    // Start some buildings!
    auto granary = ecs.entity("Granary")
        .child_of(buildingsParent)
        .set<Location>({3, 8})
        .set<BuildingUI>({3, 3, -1, 0})
        .set<Resources>({0, 0, 0})
        .add<Building>()
        ;
//        .set<flecs::components::transform::Position3>({3, 0.1, 8})
//        .set<flecs::components::geometry::Box>({2, 2, 2})
//        .set<flecs::components::graphics::Rgb>({20, 0, 0});
































    // Global
    auto ui = ecs.entity("UI Things")
        .add<Resources>()
        .add<UiPawnJobs>();



    // System to update resources
    //  A system is used here because it doesn't need to update at the same
    //  (probably faster) rate as the things are taken down
    auto updateResourceUI_sys = ecs.system<Resources>("System_Update Resource UI")
        .with<Building>()  // TODO: Want this to also consider the resources that pawns are carrying
        .tick_source(Ticks::tick_ui)
        .iter([&ui](flecs::iter it, Resources *r){
            ZoneScopedN("System_Update Resource UI");
            Resources sum{0, 0, 0};
            for (int i: it) {
                // TODO: Replace with the reflection interface?
                sum.fish += r[i].fish;
                sum.stone += r[i].stone;
                sum.wood += r[i].wood;
            }
            //std::cout << "Resources: " << std::endl;
            //std::cout << "\t fish: "  << sum.fish << std::endl;
            //std::cout << "\t stone: " << sum.stone << std::endl;
            //std::cout << "\t wood: "  << sum.wood << std::endl;
            ui.set<Resources>(sum);
        });

    // Get a count of how many pawns are doing each job
    auto unemployedQuery = ecs.query<Pawn::PawnOccupationUnemployed>();
    auto woodcutterQuery = ecs.query<Pawn::PawnOccupationWoodcutter>();
    auto updatePopulationUI_sys = ecs.system("System_Update Pawn Jobs UI")
        .tick_source(Ticks::tick_ui)
        .iter([&ui, &unemployedQuery, &woodcutterQuery](flecs::iter it){
            ZoneScopedN("System_Update Pawn Jobs UI");
            // TODO: Use the reflection interface somehow?
            UiPawnJobs sum{unemployedQuery.count(),
                           woodcutterQuery.count()};
            ui.set<UiPawnJobs>(sum);
        });
    





    
    // Initialise game
    //int map_width = 20;  // TODO: Should get this from the grid
    const float TileSize = 3.0;
    const float TileHeight = 0.5;
    const float PathHeight = 0.1;
    const float TileSpacing = 0.00;
    Game *g = ecs.get_mut<Game>();
    g->center = {0, 0, 0};//{ to_x(map_width / 2), 0, to_z(map_height / 2) };
    g->size = map_width * (TileSize + TileSpacing) + 2;
    

    // Init UI
    flecs::components::graphics::Camera camera_data = {};
    camera_data.set_up(0, 1, 0);
    camera_data.set_fov(20);
    camera_data.near_ = 1.0;
    camera_data.far_ = 100.0;
    auto camera = ecs.entity("Camera")
        .add(flecs::game::CameraController)
        .set<flecs::components::transform::Position3>({0, 8.0, -9.0})
        .set<flecs::components::transform::Rotation3>({-0.5})
        .set<flecs::components::graphics::Camera>(camera_data);

    flecs::components::graphics::DirectionalLight light_data = {};
    light_data.set_direction(0.3, -1.0, 0.5);
    light_data.set_color(0.98, 0.95, 0.8);
    light_data.intensity = 0.01;
    auto light = ecs.entity("Sun")
        .set<flecs::components::graphics::DirectionalLight>(light_data);
    

    flecs::components::gui::Canvas canvas_data = {};
    canvas_data.width = 800;
    canvas_data.height = 600;
    canvas_data.title = (char*)"TownBuilder";
    canvas_data.camera = camera.id();
    canvas_data.directional_light = light.id();
    canvas_data.ambient_light = {0.006, 0.005, 0.018};
    canvas_data.background_color = {0.15, 0.4, 0.6};
    canvas_data.fog_density = 1.0;
    ecs.entity()
        .set<flecs::components::gui::Canvas>(canvas_data);


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





    //ecs.import<Render::module>();



    ecs.app()
        .enable_rest()
        .target_fps(60)
        .run();

}