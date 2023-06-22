
#include <flecs.h>

#include <iostream>
#include <vector>


// The entity for each individual grid
struct GridCellStatic {
    int height;
};

struct GridConnected {};  // Empty type = flecs tag


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
                    .set<GridCellStatic>({0});
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



struct Position {
    double x, y;
};

struct Velocity {
    double x, y;
};







struct Walking { };

int main(int, char *[]) {
    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::monitor>(); // Enable statistics in explorer

    // Define the map
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
            snprintf(cellName, 100, "%d_%d", x, y);
            auto e = ecs.lookup(cellName);
            if (x > 0){
                // Left valid
                snprintf(otherCellName, 100, "%d_%d", x-1, y);
                auto other = ecs.lookup(otherCellName);
                e.add<GridConnected>(other);
                //grid.get(x, y).add(GridConnected, grid.get(x-1, y));
            }
            if (x <map_width-1){
                // Right valid
                flecs::entity e2 = flecs::entity(ecs, map->get(x,y));
                //flecs::entity(grid(x,y)).add<GridConnected>(e);
                //grid(x, y).add<GridConnected>(grid(x+1, y));
            }
            if (y > 0) {
                // Top valid
                //grid[x, y].add(GridConnected, grid[x, y-1]);
            }
            if (y < map_height-1){
                // Bottom valid
                //grid[x, y].add(GridConnected, grid[x, y+1]);
            }

        }
    }




























    // Put systems in
    auto move_sys = ecs.system<Position, Velocity>()
    .iter([](flecs::iter it, Position *p, Velocity *v){
        for (int i: it) {
            p[i].x += v[i].x * it.delta_time();
            p[i].y += v[i].y * it.delta_time();
        }
    });
    move_sys.add(flecs::OnUpdate);





    // Create an entity with name Bob
    auto bob = ecs.entity("Bob")
        // The set operation finds or creates a component, and sets it.
        // Components are automatically registered with the world.
        .set<Position>({10, 20})
        // The add operation adds a component without setting a value. This is
        // useful for tags, or when adding a component with its default value.
        .add<Walking>();
    bob.set<Velocity>({0.1, 0.05});


    // Get the value for the Position component
//    const Position* ptr = bob.get<Position>();
//    std::cout << "{" << ptr->x << ", " << ptr->y << "}" << "\n";
    // Overwrite the value of the Position component
//    bob.set<Position>({20, 30});

    // Create another named entity
    auto alice = ecs.entity("Alice")
        .set<Position>({10, 20});
        //.set<Velocity>({2, 4});

    // Add a tag after entity is created
    alice.add<Walking>();

    // Print all of the components the entity has. This will output:
    //    Position, Walking, (Identifier,Name)
    std::cout << "[" << alice.type().str() << "]" << "\n";

    // Remove tag
    alice.remove<Walking>();

    // Iterate all entities with Position
    ecs.each([](flecs::entity e, Position& p) {
        std::cout << e.name() << ": {" << p.x << ", " << p.y << "}" << "\n";
    });

    // Output
    //  {10, 20}
    //  [Position, Walking, (Identifier,Name)]
    //  Alice: {10, 20}
    //  Bob: {20, 30}

    while(ecs.progress(0)){};
}