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
#include "dis.hpp"
#include "pathfinding.hpp"

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


struct PtTest {
    std::shared_ptr<int> p;
};


    struct MyGrid{
        int x;
        int y;
    };



////////////////////////////////////////////////////////////////////////////////

int main(int, char *[]) {

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::stats>(); // Enable statistics in explorer

    ecs.component<std::string>()
        .opaque(flecs::String) // Opaque type that maps to string
            .serialize([](const flecs::serializer *s, const std::string *data) {
                const char *str = data->c_str();
                return s->value(flecs::String, &str); // Forward to serializer
            })
            .assign_string([](std::string* data, const char *value) {
                *data = value; // Assign new value to std::string
            });

    ecs.component<MyGrid>("MyGrid")
        .member<int>("x")
        .member<int>("y");

    
    // Logger imported first as the other modules use it on their import
    ecs.import<Logging::module>();

    ecs.import<Logging::examplemodule>();
    ecs.import<Ticks::module>();
    ecs.import<Render::module>();  // Must be before building & other modules for observers to work

    ecs.import<Coordinates::module>();
    ecs.import<Building::module>();

    //ecs.import<Map::module>();
    //ecs.import<Pawn::module>();
    //ecs.import<LogicPawn::module>();
    //ecs.import<fdis::module>();
    //ecs.import<Pathfinding::module>();

    // TODO: Determine if this is required to be done after the loggers created
    spdlog::flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));


    // Export positions to DIS - this is how playback/recording will work.
    //ecs.add<Coordinates::Converter>();
    // TODO: Something to do with the DisConnection isn't working
    //fdis::DisConnection con("localhost", 3500, 1);
    //ecs.set<fdis::DisConnection>({"localhost", 3500, 1, nullptr});
    //ecs.get_mut<fdis::DisConnection>()->connect();

    
    // Register UI components so I can see them in the flecs explorer
    /*
    ecs.component<UiPawnJobs>()
        .member<int>("unemployed")
        .member<int>("woodcutter");
    */

    // Global
    /*
    auto ui = ecs.entity("UI Things")
        .add<Building::Resources>()
        .add<UiPawnJobs>();
    */

    // Generate pawns
    //  Randomly distribute starting & target positions
    /*
    std::mt19937 rng;
    rng.seed(20231104);
    flecs::entity mapEntity;
    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();
    std::cout << "Map size: " << map->m_width << "x" << map->m_height << std::endl;
    std::cout << "Map:" << map << std::endl;
    std::uniform_int_distribution<int> xDist(0, map->m_width-1);
    std::uniform_int_distribution<int> yDist(0, map->m_height-1);
    std::uniform_real_distribution<float> speedDist(0.7, 0.9);
    std::cout << "Random distributions created" << std::endl;
    */

    /*
    constexpr int numPawns = 1;
    for (int pawnNumber=0; pawnNumber < numPawns; pawnNumber++){
        int targetX = xDist(rng);
        int targetY = yDist(rng);
        int myX = xDist(rng);
        int myY = yDist(rng);
        float speed = (float) speedDist(rng);
        char pawnName[200];
        sprintf(pawnName, "Pawn%d", pawnNumber);  // TODO: Replace with std::format
        flecs::entity pawn = ecs.entity(pawnName)
            .child_of(Pawn::pawnsParent)
            .is_a<Pawn::Pawn_Prefab>();
        std::cout << myX << ", " << myY << " -> " << targetX << ", " << targetY << std::endl;
        //pawn.set<Coordinates::Grid>({myX, myY});
        pawn.set<Coordinates::Grid>({3, 3});
        std::cout << "Pawn Grid set: " << pawn.path() << std::endl;
        pawn.set<Coordinates::Cell>({0, 0});
        std::cout << "Pawn Coordinates set: " << pawn.path() << std::endl;
        pawn.set<Coordinates::CellVelocity>({0, 0});
        pawn.add<Coordinates::GridBase>();
        pawn.set<Pawn::PawnAbilityTraits>({0, speed});
        std::cout << "First part of pawn created: " << pawn.path() << std::endl;

        // For each possible State, put the timing info in. Note that this must be
        // done before the FSM is created, otherwise it will not be able to access
        // the timing info.
        pawn.set<LogicPawn::StateTiming, LogicPawn::Alive>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::Idle>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::Walking>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::Working>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::Fleeing>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::Combat>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::Dead>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::PawnOccupationUnemployed>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::PawnOccupationWoodcutter>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::PawnWoodcutterStateWalkingTo>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::PawnWoodcutterStateReturning>({0, 0});
        pawn.set<LogicPawn::StateTiming, LogicPawn::PawnWoodcutterStateChopping>({0, 0});


        LogicPawn::Context blah{pawn.id(), ecs};  // No idea why this has to be a separate variable, but it does, so bugger it
        pawn.set<Pawn::PawnFSMContainer>({std::shared_ptr<LogicPawn::PawnFSM::Instance>(new LogicPawn::PawnFSM::Instance(blah))});




        // Set a destination
        //  This is just for test purposes
        pawn.add<Pawn::PawnPathfindingGoal>(flecs::entity(ecs, map->get(0, 0)));


    }
    */

    //logger->trace("Created pawns");

































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
        .interval(10)
        .run([&ecs](flecs::iter& it){
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

    

    


    /*
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
    */

    ecs_script_run_file(ecs, "../../src/config.flecs");
    std::cout << "Flecs script loaded" << std::endl;


    flecs::entity e = ecs.entity("test")
        .set<MyGrid>({33, 2});
    std::cout << "Test Entity Created: " << e.path() << std::endl;
    e.set<Coordinates::Grid>({32, 3});



    std::cout << "Just before run" << std::endl;
    // set the debug level so i can see the system order
    while (true) {
        ecs.progress();
        FrameMarkNamed("Frame");
    }


}