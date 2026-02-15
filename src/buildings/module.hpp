#pragma once

#include <flecs.h>

#include <stdint.h>

namespace Buildings{

struct components {
    components(flecs::world& ecs);
};

struct Building_Prefab {};
struct Granary_Prefab {};

struct Resources {
    float fish;
    float stone;
    float wood;
};

struct ResourcesLimits {
    float fish;
    float stone;
    float wood;
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