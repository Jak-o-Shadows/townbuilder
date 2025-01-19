#pragma once

// The entity for each individual grid
namespace Map {
struct GridCellStatic {
    int x, y;
    int height;
};

struct GridConnected {
    float weight;
};

}