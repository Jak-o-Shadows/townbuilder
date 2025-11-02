//#define TRACY_ON_DEMAND

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"
#include "render/module.hpp"
#include "database/module.hpp"
#include "coordinates/module.hpp"
//#include "dis/module.hpp"
#include "example/module.hpp"
#include "pawn/module.hpp"
#include "map/module.hpp"
#include "buildings/module.hpp"
#include "plugin/module.hpp"
#include "pythonEcsBinding/module.hpp"
#include "pathfinding/module.hpp"
#include "ui/module.hpp"
#include "statemachine/module.hpp"
#include "async_system.hpp"


#include "tracy_zones.hpp"

#include <flecs.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

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

// Performance tracing hooks for Flecs
static void trace_push(const char *file, size_t line, const char *name) {
    ZoneScopedN("trace_push");
    // The returned context must be stored thread-locally to handle nested zones.
    //___tracy_source_location_data srcloc = { name, __FUNCTION__, file, (uint32_t)line, 0 };
    //static thread_local std::vector<TracyCZoneCtx> tracy_context_stack;
    //tracy_context_stack.push_back(___tracy_emit_zone_begin(&srcloc, 1));
    std::cout << "Trace push: " << name << " at " << file << ":" << line << std::endl;
    ___tracy_source_location_data srcloc = { name, __FUNCTION__, file, (uint32_t)line, 0 };
    static thread_local std::vector<TracyCZoneCtx> tracy_context_stack;
    tracy_context_stack.push_back(___tracy_emit_zone_begin(&srcloc, 1));
}

static void trace_pop(const char *file, size_t line, const char *name) {
    ZoneScopedN("trace_pop");
    // Pop the context from our thread-local stack and end the Tracy zone.
    //static thread_local std::vector<TracyCZoneCtx> tracy_context_stack;
    //if (!tracy_context_stack.empty()) {
    //    ___tracy_emit_zone_end(tracy_context_stack.back());
    //    tracy_context_stack.pop_back();
    //}
    std::cout << "Trace pop: " << name << " at " << file << ":" << line << std::endl;
    static thread_local std::vector<TracyCZoneCtx> tracy_context_stack;
    if (!tracy_context_stack.empty()) {
        ___tracy_emit_zone_end(tracy_context_stack.back());
        tracy_context_stack.pop_back();
    }
}
int main(int, char *[]) {
    std::cout << "Starting main" << std::endl;

    // Link the tracing functionality into tracy
//    ecs_os_set_api_defaults();
//    ecs_os_api_t os_api = ecs_os_get_api();
//    os_api.perf_trace_push_ = trace_push;
//    os_api.perf_trace_pop_ = trace_pop;
//    ecs_os_set_api(&os_api);
//    std::cout << "Flecs performance tracing hooks set for Tracy" << std::endl;
//    std::cout << os_api.perf_trace_push_ << ", " << os_api.perf_trace_pop_ << std::endl;

    flecs::world ecs;
    ecs.set<flecs::Rest>({});// {.port=27751});  // TODO: Get multiple ports working so the plugin can listen too
    ecs.import<flecs::stats>(); // Enable statistics in explorer
    std::cout << "World created" << std::endl;





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
    ecs.import<Logging::components>();
    ecs.import<Logging::systems>();
    std::cout << "Logger imported" << std::endl;
    // Create and add the ImGui log sink component. This must be done before importing modules
    // because they may create loggers during their import, which then wouldn't be linked to the sync
    ecs.import<Render::components>();  // Suffer through not having render components log to imgui
    std::shared_ptr<Render::ImGuiLogSink_mt> imgui_sink = std::make_shared<Render::ImGuiLogSink_mt>();
    ecs.set<Render::ImGuiLogSinkComponent>({imgui_sink});
    // Add the ImGui sink to the logger sinks
    ecs.get_mut<Logging::LoggerSink>().sinks.push_back(imgui_sink);
    std::cout << "ImGui log sink created and added to LoggerSink" << std::endl;

    // Then import all the comopnents
    ecs.import<Buildings::components>();
    ecs.import<Coordinates::components>();
    ecs.import<Database::components>();
    //ecs.import<fdis::components>();
    ecs.import<Map::components>();
    ecs.import<Pathfinding::components>();
    ecs.import<Pawn::components>();
    ecs.import<Plugin::components>();
    ecs.import<Python::components>();
    ecs.import<Render::components>();
    ecs.import<Statemachine::components>();
    ecs.import<UI::components>();

    // Systems next
    ecs.import<Coordinates::systems>();
    ecs.import<Database::systems>();
    //ecs.import<fdis::systems>();
    ecs.import<Pawn::systems>();
    ecs.import<Plugin::systems>();
    ecs.import<Python::systems>();
    ecs.import<Render::systems>();
    ecs.import<Statemachine::systems>();

    // Examples are a little special
    ecs.import<Example::async>();
    ecs.import<Example::statemachine>();


    // Ticks is kinda odd one out
    ecs.import<Ticks::module>();
    std::cout << "Modules imported" << std::endl;


    // TODO: Determine if this is required to be done after the loggers created
    spdlog::flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));

    // Add an empty Connection singleton for data logging. The observer will populate it.
    ecs.set<Database::Connection>({nullptr});



    // Export positions to DIS - this is how playback/recording will work.
    //ecs.add<Coordinates::Converter>();
    // TODO: Something to do with the DisConnection isn't working
    //fdis::DisConnection con("localhost", 3500, 1);
    //ecs.set<fdis::DisConnection>({"localhost", 3500, 1, nullptr});
    //ecs.get_mut<fdis::DisConnection>()->connect();

    

    

    // Global
    /*
    auto ui = ecs.entity("UI Things")
        .add<Buildings::Resources>()
        .add<UI::PawnJobs>();
    */

    // Generate pawns
    //  Randomly distribute starting & target positions
    
    std::mt19937 rng;
    rng.seed(20231104);


    // Define the map
    //  This is defined early because it isn't properly in the ECS, so initialisation order matters mroe
    // Have a base entity - lets the map class be accssible from the ECS, and is a parent,
    //   making it show nicer in the explorer
    
    flecs::entity mapEntity = ecs.entity("map");
    //  Each cell of the map is an entity
    const int map_width = 50;
    const int map_height = 25;
    // Stored in a vector for each access
    mapEntity.emplace<Map::Grid>(map_width, map_height, &ecs, mapEntity);
    const Map::Grid& map = mapEntity.get<Map::Grid>();
    //  Initially, fully connected
    for (int x = 0; x<map_width; x++){
        for (int y = 0; y<map_height; y++){
            Map::setCellConnectivity(ecs, map, x, y, 1, 2, 6, 3, false);
        }
    }


    // Need to give the entities a parent so they show nicer in the flecs explorer
    Map::resourcesParent = ecs.entity("resources");


    // Map random-generation is VERY VERY primitive right now
    std::mt19937 rngMap;
    rngMap.seed(11223344);
    std::bernoulli_distribution treeDist(0.02);

    // Define trees
    for (int x = 0; x<map_width; x++){
        for (int y = 0; y<map_height; y++){
            // Rectangular grid is simply connected if no trees
                // TODO: This currently lets you go onto tree-cells, but not out. Is that smart?
            //flecs::entity thisCell = flecs::entity(ecs, map.get(x,y));
            if (treeDist(rngMap)){
                auto tree = ecs.entity()
                    .child_of(Map::resourcesParent)
                    .is_a<Map::Tree_Prefab>()
                    .set<Buildings::Location>({x, y})
                    .set<Buildings::Resources>({0, 100, 0})
                    .add<Buildings::NatureType>();
            }
        }
    }
    
    

    // Update the map by making the cells unaccessible
    //  TODO: This should be an observer on the children 
    //  TODO: Not really marking as inaccessible because the pathfinding currently will break
    /*resourcesParent.children([&ecs, map](flecs::entity resource){
        // Get location
        const Buildings::Location* loc = resource.get<Buildings::Location>();
        float weight = 9999999999;
        setCellConnectivity(ecs, map, loc->x, loc->y, weight, weight, weight, weight, true);
    });*/









    
    std::cout << "Map size: " << map.m_width << "x" << map.m_height << std::endl;
    //std::cout << "Map:" << map << std::endl;
    std::uniform_int_distribution<int> xDist(0, map.m_width-1);
    std::uniform_int_distribution<int> yDist(0, map.m_height-1);
    std::uniform_real_distribution<float> speedDist(3, 10);
    std::cout << "Random distributions created" << std::endl;
    

    
    constexpr int numPawns = 30;
    for (int pawnNumber=0; pawnNumber < numPawns; pawnNumber++){
        int targetX = xDist(rng);
        int targetY = yDist(rng);
        int myX = xDist(rng);
        int myY = yDist(rng);
        float speed = (float) speedDist(rng);
        char pawnName[200];
        sprintf(pawnName, "Pawn%d", pawnNumber);  // TODO: Replace with std::format
        flecs::entity pawn = ecs.entity(pawnName)
            .child_of(Pawn::pawnsParent)  // TODO: Put the IsAPawn into the prefab
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
        // Set utility curves
        pawn.set<Statemachine::CurveFile, Pawn::Alive>({"../../state1.csv"});

        Statemachine::Context blah{pawn.id(), ecs};  // No idea why this has to be a separate variable, but it does, so bugger it
        pawn.set<Pawn::PawnFSMContainer>({std::shared_ptr<Pawn::PawnFSM::Instance>(new Pawn::PawnFSM::Instance(blah))});

    }
    

    //logger->trace("Created pawns");

































    // System to update resources
    //  A system is used here because it doesn't need to update at the same
    //  (probably faster) rate as the things are taken down
    /*
    auto updateResourceUI_sys = ecs.system<Buildings::Resources>("System_Update Resource UI")
        .with<Buildings::BuildingType>()  // TODO: Want this to also consider the resources that pawns are carrying
        .tick_source(Ticks::tick_ui)
        .run([&ui](flecs::iter it){
            ZoneScopedN("System_Update Resource UI");
            while (it.next()){
                auto r = it.field<Buildings::Resources>(0);
                Buildings::Resources sum{0, 0, 0};
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
                ui.set<Buildings::Resources>(sum);
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


    





    ecs.system<Pawn::PawnFSMContainer>("Add destination")
        .term_at(0).inout()
        .interval(10)
        .tick_source(Ticks::tick_pawn_behaviour)
        .without<Pawn::Destination_Event>()
        .each([&xDist, &yDist, &rng](flecs::entity e, Pawn::PawnFSMContainer& fsmc){
            ZoneScopedN("Add destination");
            // If the pawn doesn't have a destination, give it one
            int targetX = xDist(rng);
            int targetY = yDist(rng);
            Pawn::Destination_Event dest{{targetX, targetY}, {0, 0.25}};
            // The state mchien reacting isn't working, so just force it
            // TODO: Fix this
            //fsmc.machine->changeTo<Pawn::Walking>();
            e.set<Pawn::Destination_Event>(dest);
            //fsmc.machine->react(dest);
            //std::cout << "Set destination for " << std::string(e.path()) << " to (" << targetX << ", " << targetY << ")" << std::endl;
        })
        .set_doc_brief("Set a random destination for pawns that don't have one");



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

    /*
    flecs::entity e = ecs.entity("test")
        .set<MyGrid>({33, 2});
    std::cout << "Test Entity Created: " << e.path() << std::endl;
    e.set<Coordinates::Grid>({32, 3});
    */







    std::cout << "Just before run" << std::endl;
    // set the debug level so i can see the system order
    Render::Window& w = ecs.get_mut<Render::Window>();
    while (true) {
        if (!w.alive) break;
        ecs.progress();
        FrameMarkNamed("Frame");
    }


}