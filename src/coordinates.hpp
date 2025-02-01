#pragma once

#include <flecs.h>

#include "simdis/CoordinateConverter.h"


namespace Coordinates {


struct Converter {
    simCore::CoordinateConverter converter;

    Converter() {
        converter.setReferenceOrigin(-25.23069496914944, 133.80168159420796, 0);
    }

};


struct module {
    module(flecs::world& ecs);
};

}