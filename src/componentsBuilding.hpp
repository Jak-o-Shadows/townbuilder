#pragma once

#include <flecs.h>

#include "msgLogging.hpp"


namespace Building{

struct module {
    module() = default;
    module(flecs::world& ecs, spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink);
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