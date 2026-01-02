#pragma once

#include <flecs.h>

#include <stdint.h>

namespace Buildings{

struct components {
    components(flecs::world& ecs);
};

struct Building_Prefab {};
struct Granary_Prefab {};

struct Location {
    int x;
    int y;
};

struct Resources {
    int fish;
    int stone;
    int wood;
};


struct BuildingUI {
    int sizeX;
    int sizeY;
    int doorX;
    int doorY;
};

struct BuildingType {};
struct NatureType {};

}