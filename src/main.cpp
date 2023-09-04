//#define TRACY_ON_DEMAND

#include "gridMap.hpp"
#include "componentsPawn.hpp"
#include "componentsMap.hpp"
#include "tracy_zones.hpp"

#include <flecs.h>
#include <tracy/Tracy.hpp>


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














int main(int, char *[]) {

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::monitor>(); // Enable statistics in explorer


    // Create basic timers
    // 1000 Hz should be enough for anybody
    flecs::entity tick_100_Hz = ecs.timer("Timer_100 Hz")
        .interval(0.01);

    // Pawn behaviour
    flecs::entity tick_pawn_behaviour = ecs.timer("Timer_Pawn Behaviour")
        .rate(4, tick_100_Hz);  // 4 ticks @ 100 Hz => 25 Hz


    // Modify pathfinding components
    // Each pawn can only occupy a single cell, so make exclusive
    ecs.component<PawnOccupying>().add(flecs::Exclusive);
    // Each pawn can only have a single target cell, so make exclusive
    ecs.component<PawnPathfindingGoal>().add(flecs::Exclusive);
    // Each pawn can only have a single next cell, so make exclusive
    ecs.component<PawnNextCell>().add(flecs::Exclusive);



    // Define the map
    //  Each cell of the map is an entity
    const int map_width = 128;
    const int map_height = 128;
    // Stored in a vector for each access
    flecs::entity mapEntity = ecs.entity("map");  // Need to give the map cells a parent so they show nicer in the flecs explorer
    grid *map = new grid(map_width, map_height, &ecs, mapEntity);

    // Define adjacency
    //  Initially, fully connected
    char cellName[100];  // TODO: not use the lookup
    char otherCellName[100];
    for (int x = 0; x<map_width; x++){
        for (int y = 0; y<map_height; y++){
            // Rectangular grid is simply connected
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





    // Testing
    // This works
    /*
    ecs.observer<Position>()
    .event(flecs::OnSet)
    .each([](flecs::iter& it, size_t i, Position& po) {
        std::cout << " - " << it.event().name() << ": " 
            << it.event_id().str() << ": "
            << it.entity(i).name() << ": "
            << "p: {" << po.x <<  "} " << std::endl;
    });
    */

    
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
    



    flecs::entity pawnsParent = ecs.entity("pawns");  // Need to give the pawns cells a parent so they show nicer in the flecs explorer

    // Generate pawns
    //  Randomly distribute starting & target positions
    std::mt19937 rng;
    rng.seed(20231104);
    std::uniform_int_distribution<int> xDist(0, map_width-1);
    std::uniform_int_distribution<int> yDist(0, map_height-1);
    std::uniform_real_distribution<float> speedDist(0.6, 0.8);

    constexpr int numPawns = 10;
    for (int pawnNumber=0; pawnNumber < numPawns; pawnNumber++){
        int targetX = xDist(rng);
        int targetY = yDist(rng);
        int myX = xDist(rng);
        int myY = yDist(rng);
        float speed = (float) speedDist(rng);
        char pawnName[20];
        sprintf(pawnName, "Pawn%d", pawnNumber);  // TODO: Replace with std::format
        auto pawn = ecs.entity(pawnName)
            .child_of(pawnsParent)
            .set<Position>({0.5, 0.5})
            .set<Velocity>({0, 0})
            .set<PawnAbilityTraits>({0, speed})
            .add<PawnOccupying>(flecs::entity(ecs, map->get(myX,myY)))
            .add<PawnPathfindingGoal>(flecs::entity(ecs, map->get(targetX, targetY)));

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
    
    // Frame Markers for Tracy
    if (true) {
        ecs.system("Tracy 100 Hz Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_100_Hz)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("100Hz");
        });
        ecs.system("Tracy Pawn Behaviour Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_pawn_behaviour)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("Pawn Behaviour");
        });
    }

    ecs.set_threads(4);
    while(ecs.progress(0)){
        FrameMarkNamed("Flecs Update");
    };
}