#pragma once

#include <flecs.h>

#include "simdis/CoordinateConverter.h"


namespace Coordinates {


    // Grid is the location on the map - which location it is
    struct Grid{
        int x;
        int y;
    };

    // Cell is the location within a grid square/unit - for movement
    struct Cell {
        float x;
        float y;
    };

    // CellVelocity is the velocity within a grid square/unit - for movement
    struct CellVelocity{
        float x;
        float y;
    };


    struct LLA : simCore::Coordinate {
        LLA() {
            setCoordinateSystem(simCore::CoordinateSystem::COORD_SYS_LLA);
        }
    };

    struct NED : simCore::Coordinate {
        NED() {
            setCoordinateSystem(simCore::CoordinateSystem::COORD_SYS_NED);
        }
    };

    struct ECEF : simCore::Coordinate {
        ECEF() {
            setCoordinateSystem(simCore::CoordinateSystem::COORD_SYS_ECEF);
        }
    };

    // Tags to set what the base coordinate system is for the position
    struct GridBase {};  // Motion is driven by grid changes
    struct NedBase {};  // Motion is driven by NED changes
    struct LlaBase {};  // Motion is driven by LLA changes
    struct EcefBase {};  // Motion is driven by ECEF changes


struct Converter {
    simCore::CoordinateConverter converter;

    Converter() {
        converter.setReferenceOrigin(-25.23069496914944*3.14159265/180, 133.80168159420796*3.14159265/180, 0);
    }

};


struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}