#pragma once

#include <flecs.h>

#include <stdint.h>
#include <vector>

// The entity for each individual grid
namespace Map {

struct GridCellStatic {
    int x, y;
    int height;
};

struct GridConnected {
    float weight;
};

struct components {
    components(flecs::world& ecs);
};


extern flecs::entity resourcesParent;




struct GridCell_Prefab {};

struct Tree_Prefab {};

// Have a normal vector of the the cells of the grid.
//  This makes it easier than having them all as entities, as otherwise
//  would need to query all the time just to get a cell reference
struct Grid {
public:
    Grid(int width, int height, flecs::world *ecs, flecs::entity &parent);
    void set(int32_t x, int32_t y, flecs::id_t value);
    flecs::id_t operator()(int32_t x, int32_t y) const;
    flecs::id_t get(int32_t x, int32_t y) const;  // TODO: just use an operator as above

    int m_width;
    int m_height;
private:
    std::vector<flecs::id_t> m_values;
};






// Main pathfinding function. Gives the next cell to move towards
flecs::id_t pathfind(flecs::world &ecs, const Grid* map, int currentX, int currentY, int targetX, int targetY);


void setCellConnectivity(flecs::world& ecs, const Grid* map, int x, int y, float left, float right, float up, float down, bool reversible);


}