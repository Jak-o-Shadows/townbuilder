#pragma once

#include "componentsPawn.hpp"  // TODO: Move this logic out when I figure out how to do templates better
#include "componentsMap.hpp"

#include <flecs.h>

#include <vector>
#include <iostream>


namespace Map{

struct module {
    module(flecs::world& ecs);
};

extern flecs::entity mapEntity;
extern flecs::entity resourcesParent;

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

}