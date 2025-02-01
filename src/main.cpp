//#define TRACY_ON_DEMAND

#include "gridMap.hpp"

#include "componentsPawn.hpp"
#include "componentsMap.hpp"
#include "componentsBuilding.hpp"
#include "componentsUi.hpp"
#include "logicPawn2.hpp"
#include "ticks.hpp"
#include "render.hpp"
#include "msgLogging.hpp"
#include "coordinates.hpp"

#include "tracy_zones.hpp"

#include <flecs.h>
#include <tracy/Tracy.hpp>

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









////////////////////////////////////////////////////////////////////////////////

int main(int, char *[]) {


    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::stats>(); // Enable statistics in explorer


    
    // Logger imported first as the other modules use it on their import
    ecs.import<Logging::module>();

    ecs.import<Logging::examplemodule>();
    ecs.import<Ticks::module>();
    ecs.import<Render::module>();  // Must be before building & other modules for observers to work

    ecs.import<Map::module>();
    ecs.import<Pawn::module>();
    //ecs.import<LogicPawn::module>();
    ecs.import<Building::module>();
    ecs.import<Coordinates::module>();

    // TODO: Determine if this is required to be done after the loggers created
    spdlog::flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));


    ecs.singleton<Coordinates::Converter>();


    
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
    /*
    auto updateResourceUI_sys = ecs.system<Building::Resources>("System_Update Resource UI")
        .with<Building::BuildingType>()  // TODO: Want this to also consider the resources that pawns are carrying
        .tick_source(Ticks::tick_ui)
        .run([&ui](flecs::iter it){
            ZoneScopedN("System_Update Resource UI");
            while (it.next()){
                auto r = it.field<Building::Resources>(0);
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
            }
        });
    */

    // Get a count of how many pawns are doing each job
    /*
    auto unemployedQuery = ecs.query<Pawn::PawnOccupationUnemployed>();
    auto woodcutterQuery = ecs.query<Pawn::PawnOccupationWoodcutter>();
    auto updatePopulationUI_sys = ecs.system("System_Update Pawn Jobs UI")
        .tick_source(Ticks::tick_ui)
        .run([&ui, &unemployedQuery, &woodcutterQuery](flecs::iter it){
            ZoneScopedN("System_Update Pawn Jobs UI");
            // TODO: Use the reflection interface somehow?
            UiPawnJobs sum{unemployedQuery.count(),
                           woodcutterQuery.count()};
            ui.set<UiPawnJobs>(sum);
        });
    */
    

    /*
    ecs.system("SaveWorld")
        .tick_source(Ticks::tick_100_Hz)
        .rate(100)
        .run([&ecs](flecs::iter it){
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
    */

    std::cout << "Systems in main.cpp defined" << std::endl;

    

    


    
    ecs.defer_begin();
    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();
    std::mt19937 rng;
    rng.seed(20231104);
    std::uniform_int_distribution<int> xDist(0, map->m_width-1);
    std::uniform_int_distribution<int> yDist(0, map->m_height-1);
    Pawn::pawnsParent.children([&ecs, map, &rng, &xDist, &yDist](flecs::entity pawn) {
        ZoneScopedN("Code_settingPawnTarget");
        int targetX = xDist(rng);
        int targetY = yDist(rng);
        pawn.add<Pawn::PawnPathfindingGoal>(flecs::entity(ecs, map->get(targetX, targetY)));
        });
    ecs.defer_end();
    


    ecs_script_run_file(ecs, "../../src/config.flecs");



    ecs.defer_begin();
    Pawn::pawnsParent.children([](flecs::entity pawn) {
        ZoneScopedN("");
        pawn.set<simCore::Coordinate>({simCore::CoordinateSystem::COORD_SYS_NED, simCore::Vec3(0, 0, 0)});
    });
    ecs.defer_end();





    std::cout << "Just before run" << std::endl;
    // set the debug level so i can see the system order
    while (true) {
        ecs.progress();
        FrameMarkNamed("Frame");
    }


}