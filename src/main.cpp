
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
        for (int x = 0; x < width; x ++) {
            for (int y = 0; y < height; y ++) {
                // TODO: Name them better
                char name[100];
                snprintf(name, 100, "%d_%d", x, y);
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
        .rate(4, tick_100_Hz);  // 4 ticks @ 100 Hz => 25 Hz






    // Define the map
    // Make the GridConnection transitive, so that (A, B), (B, C) = (A, C)
    // Does this make sense to do if the links have weights?
    // When I do this it crashes?????
   //ecs.component<GridConnected>().add(flecs::Transitive);

    //  Each cell of the map is an entity
    const int map_width = 4;
    const int map_height = 4;
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
                thisCell.set<GridConnected>(map->get(x-1, y), {5});
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
    int x = 2;
    int y = 2;
    flecs::entity thisCell = flecs::entity(ecs, map->get(x,y));

    // Use a loop rather than a query, as I can't figure out how to get
    //  a query to return relationships FROM a given entity (rather than to)
    int32_t index = 0;
    flecs::entity nextCell;
    while ((nextCell = thisCell.target<GridConnected>(index++))){
        std::cout << nextCell.name() << std::endl;
    }





    std::cout << "Grid Query Testing" << std::endl;
    
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
    

   // This gives all the entities that have the pairs?
//   flecs::filter<> f = ecs.filter_builder()
//    .expr("GridCellStatic, (GridConnected, *)")
//    .build();



//  flecs::Query<GridCellStatic> q = ecs.query<GridCellStatic>();
//   q.each([](GridCellStatic& a) {
//    std::cout << a.height << std::endl;
//   });






    // Generate a single pawn
    auto pawn = ecs.entity("Pawn0")
        .set<Position>({0.5, 0.5})
        .set<Velocity>({0.1, 0})
        .add<PawnOccupying>(flecs::entity(ecs, map->get(0,0)));
















    // Put systems in
    auto move_sys = ecs.system<Position, Velocity>()
    .tick_source(tick_pawn_behaviour)
    .iter([](flecs::iter it, Position *p, Velocity *v){
        std::cout << it.delta_system_time() << std::endl;
        for (int i: it) {
            p[i].x += v[i].x * it.delta_system_time();
            p[i].y += v[i].y * it.delta_system_time();

            // Figure out what cell we went to

            bool movedCell = false;
            int nextCellX;
            int nextCellY;
            if (p[i].x > 1){
                // Moved cell right
                movedCell = true;
            } else if (p[i].x < 0) {
                // Moved cell left
                movedCell = true;
            } else if (p[i].y > 1) {
                // Moved cell up
                movedCell = true;
            } else if (p[i].y < 0) {
                // Moved cell down
                movedCell = true;
            }

            //std::cout << p[i].x << ", " << p[i].y << std::endl;
            // Remove current 

        }
    });


    // Iterate all entities with Position
    ecs.each([](flecs::entity e, Position& p) {
        std::cout << e.name() << ": {" << p.x << ", " << p.y << "}" << "\n";
    });


    while(ecs.progress(0)){};
}