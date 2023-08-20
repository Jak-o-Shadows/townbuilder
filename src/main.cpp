

#include "gridMap.hpp"

#include <flecs.h>

#include <iostream>
#include <vector>


// The entity for each individual grid
struct GridCellStatic {
    int x, y;
    int height;
};

struct GridConnected {
    float weight;
};





















struct PawnLifeTraits {
    float hunger;
    float thirst;
    float cold;
    float comfort;
};

struct PawnAbilityTraits {
    float strength;
    float speed;
};

struct PawnGoals {
    flecs::id_t targetCell;  // TODO: Consider state machine?
};

struct PawnOccupying {};


struct Position {
    double x, y;
};

struct Velocity {
    double x, y;
};


struct Likes { };





struct Walking { };

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
        .rate(100, tick_100_Hz);  // 4 ticks @ 100 Hz => 25 Hz


    // Modify components
    // Each pawn can only occupy a single cell, so make exclusive
    flecs::entity e_PawnOccupying = ecs.lookup("PawnOccupying");
    e_PawnOccupying.add(flecs::Exclusive);

    // Make the GridConnection transitive, so that (A, B), (B, C) = (A, C)
    // Does this make sense to do if the links have weights?
    // When I do this it crashes?????
   //ecs.component<GridConnected>().add(flecs::Transitive);

    // Define the map
    //  Each cell of the map is an entity
    const int map_width = 20;
    const int map_height = 20;
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


    // Test out the pathfinding
    int x = 4;
    int y = 3;
    int pTargetX = 4;
    int pTargetY = 6;
    while ((x == pTargetX) || !(y == pTargetY)) {
        flecs::id_t nextCellId = pathfind(ecs, map, x, y, pTargetX, pTargetY);
        flecs::entity nextCell = flecs::entity(ecs, nextCellId);
        std::cout <<  nextCell.name() << std::endl;
        auto blah = nextCell.get<GridCellStatic>();
        x = blah->x;
        y = blah->y;
    }


    std::cout << "Grid Query Testing" << std::endl;
    
    /*
    auto q = ecs.query_builder()
        .term<GridConnected>(flecs::Wildcard)
        .build();

    //int t_var = q.find_var("t");
    
    q.iter([](flecs::iter& it) {
        auto id = it.pair(1);
        for (auto i : it) {
            std::cout << "entity " << it.entity(i) << " " << it.entity(i).name() << " has relationship "
            << id.first().name() << ", "
            << id.second().name() << std::endl;
        }
    });
    */
    
    

   // This gives all the entities that have the pairs?
//   flecs::filter<> f = ecs.filter_builder()
//    .expr("GridCellStatic, (GridConnected, *)")
//    .build();



//  flecs::Query<GridCellStatic> q = ecs.query<GridCellStatic>();
//   q.each([](GridCellStatic& a) {
//    std::cout << a.height << std::endl;
//   });


    // Observer that looks for PawnOccupying to change as to trigger an update in the pathfinding
    ecs.observer<PawnOccupying, PawnGoals>()
    .event(flecs::OnAdd)
    .event(flecs::OnSet)
    .each([](flecs::iter& it, size_t i, PawnOccupying& po, PawnGoals& pg) {
        std::cout << " - " << it.event().name() << ": " 
            << it.event_id().str() << ": "
            << it.entity(i).name() << ": "
            << "po: {" <<  "} "
            << "pg: {" <<  "}\n";
    });


    // Testing
    // This works
    ecs.observer<Position>()
    .event(flecs::OnSet)
    .each([](flecs::iter& it, size_t i, Position& po) {
        std::cout << " - " << it.event().name() << ": " 
            << it.event_id().str() << ": "
            << it.entity(i).name() << ": "
            << "po: {" << po.x <<  "} " << std::endl;
    });

    // This works for a Position, but not for a PawnOccupying. Is there a difference between components and tags?
    ecs.observer<Position>()
    .event(flecs::OnAdd)
    .each([](flecs::iter& it, size_t i, Position& p) {
        std::cout << " - " << it.event().name() << ": " 
            << it.event_id().str() << ": "
            << it.entity(i).name() << ": "
            << "p: {" <<  "} " << std::endl;
    });



    flecs::entity pawnsParent = ecs.entity("pawns");  // Need to give the pawns cells a parent so they show nicer in the flecs explorer
    
    // Generate a single pawn
    int targetX = 15;
    int targetY = 8;
    int myX = 0;
    int myY = 0;
    auto pawn = ecs.entity("Pawn0")
        .child_of(pawnsParent)
        .set<Position>({0.5, 0.5})
        .set<Velocity>({0.1, 0})
        .add<PawnOccupying>(flecs::entity(ecs, map->get(myX,myY)))
        .add<PawnGoals>(flecs::entity(ecs, map->get(targetX, targetY)));

    /*



    auto queryTest = ecs.query_builder<PawnOccupying, Position>("Pawn Occupying Test Query")
        .term_at(1).second(flecs::Wildcard)  // Change first argument to (PawnOccupying, *)
        .build();
    queryTest.each([&map, &ecs](flecs::iter& it, size_t index, PawnOccupying& pawnOccupying, Position& p) {
        auto e = it.entity(index);
        auto thisCell = it.pair(1).second();
        
        std::cout << "querytest " << e.name() << " Occupying " << thisCell.name() << std::endl;

        std::cout << "\t " << p.x << std::endl;

            bool movedCell = false;
            flecs::entity nextCell;
            if (p[i].x > 1){
                // Moved cell right
                nextCell = map->get(thisCell.x+1, thisCell.y);
                p[i].x = 0;
                movedCell = true;
            } else if (p[i].x < 0) {
                // Moved cell left
                nextCell = map->get(thisCell.x-1, thisCell.y);
                p[i].x = 1;
                movedCell = true;
            } else if (p[i].y > 1) {
                // Moved cell up
                nextCell = map->get(thisCell.x, thisCell.y+1);
                p[i].y = 0;
                movedCell = true;
            } else if (p[i].y < 0) {
                // Moved cell down
                nextCell = map->get(thisCell.x, thisCell.y-1);
                p[i].y = 1;
                movedCell = true;
            }

        if (movedCell==true){
            e.add<PawnOccupying>(flecs::entity(ecs, nextCell));
        }


    });

    */





    // Put systems in
    auto move_sys = ecs.system<Position, Velocity>("System_Intra Cell Movement")
    .tick_source(tick_pawn_behaviour)
    .iter([](flecs::iter it, Position *p, Velocity *v){
        for (int i: it) {
            p[i].x += v[i].x * it.delta_system_time();
            p[i].y += v[i].y * it.delta_system_time();
        }
    });

    /*
    auto cell_move_sys = ecs.system<PawnOccupying>("Cell Moveover")
        .tick_source(tick_pawn_behaviour)
        .iter([](flecs::iter it, PawnOccupying *po){
            std::cout<<"Pawn Occuping" << std::endl;
        });
    */

   /* Look for a component to know to update it
    auto cell_move_sys = ecs.system<PawnPathfindingUpdateRequired>("Pawn Pathfinding Update")
        .tick_source(tick_pawn_behaviour)
        .iter([](flecs::iter it, PawnPathfindingUpdateRequired *pawnPathfindingUpdate){
            std::cout<<"Pawn Update Required" << std::endl;
        });
    */

    // Iterate all entities with Position
    ecs.each([](flecs::entity e, Position& p) {
        std::cout << e.name() << ": {" << p.x << ", " << p.y << "}" << "\n";
    });


    while(ecs.progress(0)){};
}