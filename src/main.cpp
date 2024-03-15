//#define TRACY_ON_DEMAND

#include "gridMap.hpp"

#include "componentsPawn.hpp"
#include "componentsMap.hpp"
#include "componentsBuilding.hpp"
#include "componentsUi.hpp"
#include "logicPawn2.hpp"


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
/*
int main() {
	// shared data storage instance
	Context context;

	FSM::Instance machine{context};

	while (machine.isActive<Off>() == false)
		machine.update();

	return 0;
}
*/





struct PawnFSMContainer {
    std::unique_ptr<LogicPawn::PawnFSM::Instance> machine;
};





























































































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
    
    ecs.import<LogicPawn::module>();




    // Create basic timers
    // 1000 Hz should be enough for anybody
    flecs::entity tick_100_Hz = ecs.timer("Timer_100 Hz")
        .interval(0.01);
    // Pawn behaviour
    flecs::entity tick_pawn_behaviour = ecs.timer("Timer_Pawn Behaviour")
        .rate(4, tick_100_Hz);  // 4 ticks @ 100 Hz => 25 Hz
    // UI Updates
    flecs::entity tick_ui = ecs.timer("Timer_UI Update")
        .rate(8, tick_100_Hz);  // 8 ticks @ 100 Hz => 12.5 Hz
    // Render update
    flecs::entity tick_render = ecs.timer("Timer_Render Update")
        .rate(2, tick_100_Hz);  // 2 ticks @ 100 Hz => 50 Hz





    // Modify pathfinding components
    // Each pawn can only occupy a single cell, so make exclusive
    ecs.component<PawnOccupying>().add(flecs::Exclusive);
    // Each pawn can only have a single target cell, so make exclusive
    ecs.component<PawnPathfindingGoal>().add(flecs::Exclusive);
    // Each pawn can only have a single next cell, so make exclusive
    ecs.component<PawnNextCell>().add(flecs::Exclusive);


    // Need to give the entities a parent so they show nicer in the flecs explorer
    flecs::entity pawnsParent = ecs.entity("pawns");
    flecs::entity resourcesParent = ecs.entity("resources");
    flecs::entity buildingsParent = ecs.entity("buildings");
    flecs::entity mapEntity = ecs.entity("map");




    // Register components with reflection data
    ecs.component<PawnLifeTraits>()
        .member<float>("hunger")
        .member<float>("thirst")
        .member<float>("cold")
        .member<float>("comfort");
    ecs.component<PawnAbilityTraits>()
        .member<float>("strength")
        .member<float>("speed");
    ecs.component<Resources>()
        .member<int>("fish")
        .member<int>("stone")
        .member<int>("wood");
    
    // Register UI components so I can see them
    ecs.component<UiPawnJobs>()
        .member<int>("unemployed")
        .member<int>("woodcutter");











    // State Machines
    //  Each 
    ecs.component<PawnWoodcutterState>().add(flecs::Exclusive);




    // Define the map
    //  Each cell of the map is an entity
    std::mt19937 rngMap;
    rngMap.seed(641331);
    const int map_width = 20;
    const int map_height = 20;
    // Stored in a vector for each access
    grid *map = new grid(map_width, map_height, &ecs, mapEntity);

    // Map random-generation is VERY VERY primitive right now
    std::bernoulli_distribution treeDist(0.2);

    // Define adjacency
    //  Initially, fully connected
    char cellName[100];  // TODO: not use the lookup
    char otherCellName[100];
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
                    .set<flecs::components::transform::Position3>({(float) x, 1, (float) y})
                    .set<flecs::components::graphics::Rgb>({0, 255, 0})
                    .set<flecs::components::geometry::Box>({0.1, 0.5, 0.1});
            } else {
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
    }







    
    ecs.observer()
        .event(flecs::OnAdd)
        .with<PawnOccupying>(flecs::Wildcard)
        .with<PawnPathfindingGoal>(flecs::Wildcard)
        .each([&map, &ecs](flecs::entity e){
            ZoneScopedN(ts_PawnPathfindingUpdate);
            //std::cout << "Pathfinding Update: for " <<  e.name() << std::endl;
            flecs::entity currentCell = e.target<PawnOccupying>();
            flecs::entity targetCell = e.target<PawnPathfindingGoal>();
            //std::cout << "\t " << currentCell.name() << " to " << targetCell.name() << std::endl;
            // From the current cell, and the target cell, pathfind to find the next cell
            //  First convert the entity data into x, y
            //      Current Cell
            auto currentStatic = currentCell.get<GridCellStatic>();
            int x = currentStatic->x;
            int y = currentStatic->y;
            //      Target Cell
            auto targetStatic = targetCell.get<GridCellStatic>();
            int targetX = targetStatic->x;
            int targetY = targetStatic->y;
            if ((targetX == x) && (targetY == y)) {
                // No need to pathfind - stay still
                //std::cout << "\t" << e.name() << " finished!" << std::endl;
                e.set<Velocity>({0, 0});
            } else {
                //  Begin Pathfinding
                flecs::id_t nextCellId = pathfind(ecs, map, x, y, targetX, targetY);
                flecs::entity nextCell = flecs::entity(ecs, nextCellId);
                // Set the next cell
                e.add<PawnNextCell>(nextCell);
                // Calculate velocity to get to next cell
                // TODO: This might need to be smarter if I have non-uniform cells
                // TODO: This is assuming from middle of cell to middle of cell, not from edge to edge
                auto nextStatic = nextCell.get<GridCellStatic>();
                const PawnAbilityTraits *pawnAbilityTraits = e.get<PawnAbilityTraits>();
                float speed = pawnAbilityTraits->speed;
                float vx = (nextStatic->x - x)*speed;
                float vy = (nextStatic->y -y)*speed;
                //std::cout << "\t Velocity To: " << vx << ", " << vy << std::endl;
                e.set<Velocity>({vx, vy});
            }
        });
    





    // Generate pawns
    //  Randomly distribute starting & target positions
    std::mt19937 rng;
    rng.seed(20231104);
    std::uniform_int_distribution<int> xDist(0, map_width-1);
    std::uniform_int_distribution<int> yDist(0, map_height-1);
    std::uniform_real_distribution<float> speedDist(0.1, 0.3);

    constexpr int numPawns = 20;
    for (int pawnNumber=0; pawnNumber < numPawns; pawnNumber++){
        int targetX = xDist(rng);
        int targetY = yDist(rng);
        int myX = xDist(rng);
        int myY = yDist(rng);
        float speed = (float) speedDist(rng);
        char pawnName[200];
        sprintf(pawnName, "Pawn%d", pawnNumber);  // TODO: Replace with std::format
        auto pawn = ecs.entity(pawnName)
            .child_of(pawnsParent)
            .set<Position>({0.5, 0.5})
            .set<Velocity>({0, 0})
            .set<PawnAbilityTraits>({0, speed})
            .add<PawnOccupationWoodcutter>()
            .add<PawnWoodcutterState>(ecs.component<PawnWoodcutterStateIdle>())
            .add<PawnOccupying>(flecs::entity(ecs, map->get(myX,myY)))
            .add<PawnPathfindingGoal>(flecs::entity(ecs, map->get(targetX, targetY)))
            .set<flecs::components::transform::Position3>({(float) myX, 0.1, (float) myY})
            .set<flecs::components::geometry::Box>({0.1, 0.8, 0.1})
            .set<flecs::components::graphics::Rgb>({165, 42, 42});
        //pawn.add<PawnFSMContainer>({PawnFSM::Instance{pawn.id()}});


        //PawnFSM::Instance machine{blah};
        //pawn.set<PawnFSMContainer>({PawnFSM::Instance{blah}});
        //std::unique_ptr<PawnFSM::Instance> ptr(new PawnFSM::Instance(blah));// = std::make_unique<PawnFSM::Instance>(machine);
        pawn.set<PawnFSMContainer>({std::unique_ptr<LogicPawn::PawnFSM::Instance>(new LogicPawn::PawnFSM::Instance{LogicPawn::Context{pawn.id(), ecs}})});




        //flecs::entity_to_json_desc_t desc;
        //desc.serialize_path = true;
        //desc.serialize_values = true;
        //std::cout << pawn.to_json(&desc) << "\n";

    }
    std::cout << "Pawns Created" << std::endl;






    // Put systems in
    auto move_sys = ecs.system<Position, Velocity>("System_Intra Cell Movement")
    .tick_source(tick_pawn_behaviour)
    .iter([](flecs::iter it, Position *p, Velocity *v){
        for (int i: it) {
            p[i].x += v[i].x * it.delta_system_time();
            p[i].y += v[i].y * it.delta_system_time();
            //std::cout << p[i].x << ", " << p[i].y << " @ " << v[i].x << ", " << v[i].y << std::endl;
        }
    });
    
    auto moveCell_sys = ecs.system<Position>("System Between Cell Movement")
    .with<PawnNextCell>(flecs::Wildcard)
    .tick_source(tick_pawn_behaviour)
    .multi_threaded()
    .iter([](flecs::iter it, Position *p){
        ZoneScopedN("System Between Cell Movement");
        for (int i: it){
            //std::cout << "Checking Cell Moveover" << std::endl;
            flecs::entity e = it.entity(i);
            bool movedCell = false;
            // Assume that the velocity is correct, and if we move over any boundary we're there
            if (p[i].x > 1){
                movedCell = true;
                p[i].x = 0;
            } else if (p[i].x < 0) {
                // Moved cell left
                movedCell = true;
                p[i].x = 1;
            } 
            if (p[i].y > 1) {
                // Moved cell up
                movedCell = true;
                p[i].y = 0;
            } else if (p[i].y < 0) {
                // Moved cell down
                movedCell = true;
                p[i].y = 1;
            }

            if (movedCell){
                flecs::entity nextCell = e.target<PawnNextCell>();
                //std::cout << " Moved To " << nextCell.name() << std::endl;
                // Uupdate currently occupying cell
                e.add<PawnOccupying>(nextCell);
                // Update overall goal to force pathfinding update
                flecs::entity goalCell = e.target<PawnPathfindingGoal>();
                e.add<PawnPathfindingGoal>(goalCell);
            }
        }
    });

    auto updatePawnRenderLocation_sys = ecs.system<Position, flecs::components::transform::Position3>("Update Pawn Render Location")
    .tick_source(tick_render)
    .with<PawnOccupying>(flecs::Wildcard)
    .each([](flecs::entity pawn, Position& p, flecs::components::transform::Position3& renderPos){
        // Get cell from pawn occupying
        flecs::entity currentCell = pawn.target<PawnOccupying>();
        const GridCellStatic* loc = currentCell.get<GridCellStatic>();
        renderPos.x = loc->x + p.x;
        renderPos.z = loc->y + p.y;
    });




    // Start some buildings!
    auto granary = ecs.entity("Granary")
        .child_of(buildingsParent)
        .set<Location>({3, 8})
        .set<BuildingUI>({3, 3, -1, 0})
        .set<Resources>({0, 0, 0})
        .add<Building>()
        .set<flecs::components::transform::Position3>({3, 0.1, 8})
        .set<flecs::components::geometry::Box>({2, 2, 2})
        .set<flecs::components::graphics::Rgb>({20, 0, 0});






    // State actions
    auto blah_sys = ecs.system<>("ASDF")
        .with<PawnWoodcutterState>(ecs.component<PawnWoodcutterStateIdle>())
        .tick_source(tick_pawn_behaviour)
        .multi_threaded()
        .iter([](flecs::iter it){
            ZoneScopedN("Pawn Woodctuter Idle State Actions");
            for (int i: it){
                flecs::entity e = it.entity(i);
                // If they are idle, get them to find the nearest wood and path-find towards it
                
            }
    });

























    // Global
    auto ui = ecs.entity("UI Things")
        .add<Resources>()
        .add<UiPawnJobs>();



    // System to update resources
    //  A system is used here because it doesn't need to update at the same
    //  (probably faster) rate as the things are taken down
    auto updateResourceUI_sys = ecs.system<Resources>("System_Update Resource UI")
        .with<Building>()  // TODO: Want this to also consider the resources that pawns are carrying
        .tick_source(tick_ui)
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
    auto unemployedQuery = ecs.query<PawnOccupationUnemployed>();
    auto woodcutterQuery = ecs.query<PawnOccupationWoodcutter>();
    auto updatePopulationUI_sys = ecs.system("System_Update Pawn Jobs UI")
        .tick_source(tick_ui)
        .iter([&ui, &unemployedQuery, &woodcutterQuery](flecs::iter it){
            ZoneScopedN("System_Update Pawn Jobs UI");
            // TODO: Use the reflection interface somehow?
            UiPawnJobs sum{unemployedQuery.count(),
                           woodcutterQuery.count()};
            ui.set<UiPawnJobs>(sum);
        });
    

    // Frame Markers for Tracy
    if (true) {
        ecs.system("Tracy 100 Hz Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_100_Hz)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("100 Hz");
        });
        ecs.system("Tracy Pawn Behaviour Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_pawn_behaviour)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("Pawn Behaviour");
        });
        ecs.system("Tracy UI Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_ui)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("Tick UI");
        });
    }



    
    // Initialise game
    static const float TileSize = 3.0;
    static const float TileHeight = 0.5;
    static const float PathHeight = 0.1;
    static const float TileSpacing = 0.00;
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







    ecs.app()
        .enable_rest()
        .target_fps(60)
        .run();


    /*
    ecs.set_threads(4);
    while(ecs.progress(0)){
        FrameMarkNamed("Flecs Update");
    };
    */
}