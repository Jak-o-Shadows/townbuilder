#pragma once

#include <flecs.h>


namespace Building{

struct module {
    module(flecs::world& ecs);
};

extern flecs::entity buildingsParent;


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