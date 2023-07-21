
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


// Have a normal vector of the the cells of the grid.
//  This makes it easier than having them all as entities, as otherwise
//  would need to query all the time just to get a cell reference
class grid {
public:
    grid(int width, int height, flecs::world *ecs)
        : m_width(width)
        , m_height(height) 
    { 
        for (int y = 0; y < height; y ++) {
            for (int x = 0; x < width; x ++) {
                // TODO: Name them better
                char name[100];
                snprintf(name, 100, "%d_%d", x, y);
                //std::cout << x << ", " << y << " name is " << name << std::endl;
                auto cell = ecs->entity(name)
                    // The set operation finds or creates a component, and sets it.
                    // Components are automatically registered with the world.
                    .set<GridCellStatic>({x, y, 10*x + y});
                m_values.push_back(ecs->id(cell));
            }
        }
    }

    void set(int32_t x, int32_t y, flecs::id_t value) {
        m_values[y * m_width + x] = value;
    }

    flecs::id_t operator()(int32_t x, int32_t y) {
        return m_values[y * m_width + x];
    }

    flecs::id_t get(int32_t x, int32_t y) {  // TODO: just use an operator as above
        return m_values[y * m_width + x];
    }

private:
    int m_width;
    int m_height;
    std::vector<flecs::id_t> m_values;
};

template< class T >
class templateGrid {
public:
    templateGrid(int width, int height)
        : m_width(width)
        , m_height(height)
        , m_values(width*height)
    { }

    void set(int32_t x, int32_t y, T value) {
        //std::cout << "\t\t\tSetting " << x << ", " << y << " from " << y * m_width + x << " to " << value << std::endl;
        m_values[y * m_width + x] = value;
    }

    T operator()(int32_t x, int32_t y) {
        return m_values[y * m_width + x];
    }

    T get(int32_t x, int32_t y) {  // TODO: just use an operator as above
        //std::cout << "\t\t\tGetting " << x << ", " << y << " from " << y * m_width + x << std::endl;
        return m_values[y * m_width + x];
    }

private:
    int m_width;
    int m_height;
    std::vector<T> m_values;
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
    const int map_width = 10;
    const int map_height = 10;
    // Stored in a vector for each access
    grid *map = new grid(map_width, map_height, &ecs);

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


    // Get the connections for a given cell
    int x = 4;
    int y = 3;
    flecs::entity thisCell = flecs::entity(ecs, map->get(x,y));

    // Use a loop rather than a query, as I can't figure out how to get
    //  a query to return relationships FROM a given entity (rather than to)
    int32_t index = 0;
    flecs::entity nextCell;
    while ((nextCell = thisCell.target<GridConnected>(index++))){
        std::cout << nextCell.name() << std::endl;
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


    // Path-finding testing
    std::cout << "Path-Finding Testing" << std::endl;
    // Define variables
    templateGrid<bool> visitedGrid = templateGrid<bool>(map_width, map_height);
    templateGrid<float> costGrid = templateGrid<float>(map_width, map_height);
    templateGrid<flecs::id_t> prevGrid = templateGrid<flecs::id_t>(map_width, map_height);
    // Initialise start values
    for (int x=0;x<map_width;x++){
        for (int y=0; y<map_height;y++){
            visitedGrid.set(x, y, false);
            costGrid.set(x, y, 999999999); // TODO: Swap to an inf
        }
    }

    // Begin pathfinding
    x = 2;
    y = 2;

    thisCell = flecs::entity(ecs, map->get(x,y));
    costGrid.set(x, y, 0);  // First cell has zero cost
    
    //while (true) {
    for (int i=0; i<500; i++){
        std::cout << thisCell.name() << " (" << x << ", " << y << ")" << std::endl;
        // Mark the current grid as visisted
        visitedGrid.set(x, y, true);
        //std::cout << "\tVisited self? " << visitedGrid.get(x, y) << std::endl;
        //std::cout << "\tVisited 8, 7? " << visitedGrid.get(8, 7) << std::endl;

        // Get the the links from the current cell
        auto queryTest = ecs.query_builder<GridConnected, GridCellStatic>("Getting next cell query")
            .term_at(1).second(thisCell)  // Change first argument to (PawnOccupying, *)
            .build();
        queryTest.each([&map, &ecs, &costGrid, &visitedGrid, &prevGrid, &thisCell, &x, &y](flecs::iter& it, size_t index, GridConnected& conn, GridCellStatic& nextCellStatic) {
            auto e = it.entity(index);
            auto thisCell = it.pair(1).second();

            std::cout << "\t" << nextCellStatic.x << ", " << nextCellStatic.y << std::endl;
            //std::cout << "\t" << conn.weight << std::endl;

            auto nextX = nextCellStatic.x;
            auto nextY = nextCellStatic.y;
            if (!visitedGrid.get(nextX, nextY)) {
                // IF the neighbouring cell hasn't already been visited,
                //  check if it's cost an be lowered
                std::cout << "\t\tUnvisisted" << std::endl;
                auto potentialCost = conn.weight + costGrid.get(x, y);
                if (potentialCost < costGrid.get(nextX, nextY)) {
                    // If the cost can be lowered, lower it, and set it's previous cell
                    //  to the current cell
                    std::cout << "\t\t" << "Updating cost to " << potentialCost << std::endl;
                    costGrid.set(nextX, nextY, potentialCost);
                    prevGrid.set(nextX, nextY, thisCell);
                }
            }
        });
        // Select the next cell that has the lowest cost that hasn't been visited yet
        //  TODO: This really should be more efficient by using some kind of priority heap
        float minCost = 9999999999;
        int minCostX;
        int minCostY;
        for (int x=0;x<map_width;x++){
            for (int y=0; y<map_height;y++){
                if (!visitedGrid.get(x, y)) {
                    if (costGrid.get(x,y) < minCost) {
                        minCost = costGrid.get(x,y);
                        minCostX = x;
                        minCostY = y;
                    }
                }
            }
        }

        if (minCost != 9999999999) {
            x = minCostX;
            y = minCostY;
            thisCell = flecs::entity(ecs, map->get(x,y));
        } else {
            break;
        }

    }

    // Print it


    // Work backwards from the target cell to get the path
    for (int y=0; y<map_height;y++){
        for (int x=0;x<map_width;x++){
            if (costGrid.get(x, y) > 0) {
                auto prev = flecs::entity(ecs, prevGrid.get(x, y));
                auto blah = prev.get<GridCellStatic>();
                std::cout << "(" << blah->x << "," << blah->y << ") ";
            } else {
                // Starting cell does't have a previous
                std::cout << "[" << x << "," << y << "] ";
            }
        }
        std::cout << std::endl;
    }
    x = 8;
    y = 8;
    for (int maxIter = 0; maxIter <= map_width*map_height; maxIter++) { 
        auto next =  flecs::entity(ecs, prevGrid.get(x, y));
        std::cout << next.name() << std::endl;
        auto blah = next.get<GridCellStatic>();
        x = blah->x;
        y = blah->y;
        if ((x == 2) && (y == 2)){
            break;
        }
    }


    /*
    // Generate a single pawn
    auto pawn = ecs.entity("Pawn0")
        .set<Position>({0.5, 0.5})
        .set<Velocity>({0.1, 0})
        .add<PawnOccupying>(flecs::entity(ecs, map->get(0,0)));








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

    // Iterate all entities with Position
    ecs.each([](flecs::entity e, Position& p) {
        std::cout << e.name() << ": {" << p.x << ", " << p.y << "}" << "\n";
    });


    while(ecs.progress(0)){};
}