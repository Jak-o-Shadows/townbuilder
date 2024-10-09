//#define TRACY_ON_DEMAND

#include "gridMap.hpp"

#include "componentsPawn.hpp"
#include "componentsMap.hpp"
#include "componentsBuilding.hpp"
#include "componentsUi.hpp"
#include "logicPawn2.hpp"
#include "ticks.hpp"
#include "render.hpp"

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

#include <iostream>
#include <fstream>
#include <format>
#include <vector>
#include <random>


#define HFSM2_ENABLE_STRUCTURE_REPORT
#include <hfsm2/machine.hpp>

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


    





    ecs.import<Map::module>();
    ecs.import<LogicPawn::module>();
    ecs.import<Pawn::module>();
    ecs.import<Ticks::module>();
    ecs.import<Building::module>();




    
    // Register UI components so I can see them in the flecs explorer
    ecs.component<UiPawnJobs>()
        .member<int>("unemployed")
        .member<int>("woodcutter");


    // Global
    auto ui = ecs.entity("UI Things")
        .add<Building::Resources>()
        .add<UiPawnJobs>();



    // System to update resources
    //  A system is used here because it doesn't need to update at the same
    //  (probably faster) rate as the things are taken down
    auto updateResourceUI_sys = ecs.system<Building::Resources>("System_Update Resource UI")
        .with<Building::BuildingType>()  // TODO: Want this to also consider the resources that pawns are carrying
        .tick_source(Ticks::tick_ui)
        .iter([&ui](flecs::iter it, Building::Resources *r){
            ZoneScopedN("System_Update Resource UI");
            Building::Resources sum{0, 0, 0};
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
            ui.set<Building::Resources>(sum);
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
    

    ecs.system("SaveWorld")
        .tick_source(Ticks::tick_100_Hz)
        .rate(100)
        .iter([&ecs](flecs::iter it){
            ZoneScopedN("SaveWorld");
            flecs::string json = ecs.to_json();

            std::ofstream outfile(std::format("world_{}.json", it.world().get_info()->world_time_total));
            // Check if the file is open
            if (!outfile.is_open()) {
                std::cout << "Failed to open file for writing." << std::endl;
            } else {
                outfile << json;
                outfile.close();
            }
        });


    
    // Initialise game
    const float TileSize = 3.0;
    const float TileHeight = 0.5;
    const float PathHeight = 0.1;
    const float TileSpacing = 0.00;
    Game *g = ecs.get_mut<Game>();
    g->center = {0, 0, 0};//{ to_x(map_width / 2), 0, to_z(map_height / 2) };
    // Get the map entity back out for working with for the moment
    std::shared_ptr<Map::grid> map = Map::mapEntity.get<Map::MapContainer>()->map;
    int map_width = map->m_width;
    int map_height = map->m_height;
    g->size = map_width * (TileSize + TileSpacing) + 2;
    
    // Cannot figure out how to move these to render - so stuff it
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


    









    ecs.import<Render::module>();



    ecs.app()
        .enable_rest()
        .threads(4)
        .delta_time(1.0/200.0)  // Setting a fixed framerate causes the internal clock to make sense
        .run();

}