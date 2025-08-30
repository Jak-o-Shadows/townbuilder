#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include <flecs.h>
#include <KDIS/KDefines.hpp>
#include <KDIS/PDU/Header.hpp>
#include <KDIS/Network/Connection.hpp>

namespace fdis {

struct DisPdus {
    std::vector<std::unique_ptr<KDIS::PDU::Header>> pdusSinceLastUpdate;
};


struct DisConnection{
    KDIS::KString sendAddress;
    KDIS::KUINT32 port;
    KDIS::KUINT8 exerciseID;
    std::unique_ptr<KDIS::NETWORK::Connection> con;
};



struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}