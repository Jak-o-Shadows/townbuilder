
#include "gridMap.hpp"

#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"




// Have a normal vector of the the cells of the grid.
//  This makes it easier than having them all as entities, as otherwise
//  would need to query all the time just to get a cell reference
grid::grid(int width, int height, flecs::world *ecs, flecs::entity &parent)
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
                    .set<GridCellStatic>({x, y, 10*x + y})
                    .set<flecs::components::geometry::Box>({1, 0, 1})
                    .set<flecs::components::transform::Position3>({(float) x, 0, (float) y})
                    .set<flecs::components::graphics::Rgb>({(float) x, (float) y, 55})
                    .child_of(parent);  // Need to give the map cells a parent so they show nicer in the flecs explorer
                m_values.push_back(ecs->id(cell));
            }
        }
    }

void grid::set(int32_t x, int32_t y, flecs::id_t value) {
    m_values[y * m_width + x] = value;
}

flecs::id_t grid::operator()(int32_t x, int32_t y) {
    return m_values[y * m_width + x];
}

flecs::id_t grid::get(int32_t x, int32_t y) {  // TODO: just use an operator as above
    return m_values[y * m_width + x];
}



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

//private:
    int m_width;
    int m_height;
    std::vector<T> m_values;
};



flecs::id_t pathfind(flecs::world &ecs, std::shared_ptr<grid> map, int currentX, int currentY, int targetX, int targetY){


    int x = currentX;
    int y = currentY;

    // Alias these to avoid having to change the working bit of code.
    int map_width = map->m_width;
    int map_height = map->m_height;
    int maxIter = map_width * map_height -1;


    //std::cout << "Path-Finding Testing" << std::endl;
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

    // Start performing Dijkstra's Algorithm
    auto thisCell = flecs::entity(ecs, map->get(x,y));
    costGrid.set(x, y, 0);  // First cell has zero cost
    for (int i=0; i<maxIter; i++){
        //std::cout << thisCell.name() << " (" << x << ", " << y << ")" << std::endl;
        // Mark the current grid as visisted
        visitedGrid.set(x, y, true);
        //std::cout << "\tVisited self? " << visitedGrid.get(x, y) << std::endl;
        //std::cout << "\tVisited 8, 7? " << visitedGrid.get(8, 7) << std::endl;

        // Get the the links from the current cell. Use a filter because it doesn't need to last
       auto queryTest = ecs.filter_builder<GridConnected, GridCellStatic>("Getting next cell query")
            .term_at(1).second(thisCell)  // Change first argument to (PawnOccupying, *)
            .build();
        queryTest.each([&map, &ecs, &costGrid, &visitedGrid, &prevGrid, &thisCell, &x, &y](flecs::iter& it, size_t index, GridConnected& conn, GridCellStatic& nextCellStatic) {
            auto e = it.entity(index);
            auto thisCell = it.pair(1).second();

            //std::cout << "\t" << nextCellStatic.x << ", " << nextCellStatic.y << std::endl;
            //std::cout << "\t" << conn.weight << std::endl;

            auto nextX = nextCellStatic.x;
            auto nextY = nextCellStatic.y;
            if (!visitedGrid.get(nextX, nextY)) {
                // IF the neighbouring cell hasn't already been visited,
                //  check if it's cost an be lowered
                //std::cout << "\t\tUnvisisted" << std::endl;
                auto potentialCost = conn.weight + costGrid.get(x, y);
                if (potentialCost < costGrid.get(nextX, nextY)) {
                    // If the cost can be lowered, lower it, and set it's previous cell
                    //  to the current cell
                    //std::cout << "\t\t" << "Updating cost to " << potentialCost << std::endl;
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
            // No remaining cells
            break;
        }
    }
    //std::cout << "Dijkstra's Completed for start cell (" << currentX << ", " << currentY << ")" << std::endl;

    // Work backwards from the target cell to get the path
    x = targetX;
    y = targetY;
    flecs::id_t thisSpot = map->get(x, y);
    flecs::id_t prevSpot = thisSpot;  // Handle final case where there is just one link
    for (int i = 0; i <= maxIter; i++) { // Theoretically the path could use all cells
        thisSpot = prevGrid.get(x, y);
        //std::cout << x << ", " << y << " of id" << thisSpot << std::endl;
        auto blah = flecs::entity(ecs, thisSpot).get<GridCellStatic>();
        x = blah->x;
        y = blah->y;
        if ((x == currentX) && (y == currentY)){
            break;
        }
        prevSpot = thisSpot;
    }
    //std::cout << prevSpot << flecs::entity(ecs, prevSpot).name() << std::endl;
    return prevSpot;

}

