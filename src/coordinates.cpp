#include <tracy/Tracy.hpp>

#include "coordinates.hpp"

#include "msgLogging.hpp"
#include "componentsPawn.hpp"

namespace Coordinates {

// Handle extern entities
flecs::entity pawnsParent;
std::shared_ptr<spdlog::logger> logger;


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");

    // Register Simdis components
    ecs.component<simCore::Vec3>()
        .member<double>("x")
        .member<double>("y")
        .member<double>("z");

    ecs.component<simCore::CoordinateSystem>()
        .constant("COORD_SYS_NONE", simCore::COORD_SYS_NONE)
        .constant("COORD_SYS_NED", simCore::COORD_SYS_NED)
        .constant("COORD_SYS_NWU", simCore::COORD_SYS_NWU)
        .constant("COORD_SYS_ENU", simCore::COORD_SYS_ENU)
        .constant("COORD_SYS_LLA", simCore::COORD_SYS_LLA)
        .constant("COORD_SYS_ECEF", simCore::COORD_SYS_ECEF)
        .constant("COORD_SYS_ECI", simCore::COORD_SYS_ECI)
        .constant("COORD_SYS_XEAST", simCore::COORD_SYS_XEAST)
        .constant("COORD_SYS_GTP", simCore::COORD_SYS_GTP)
        .constant("COORD_SYS_MAX", simCore::COORD_SYS_MAX);


    ecs.component<simCore::Coordinate>()
        //.member<simCore::CoordinateSystem>("system")
        .member<simCore::Vec3>("pos")
        .member<simCore::Vec3>("vel")
        .member<simCore::Vec3>("ori")
        .member<simCore::Vec3>("acc")
        .member<double>("elapsedEciTime")
        .member<bool>("hasVel")
        .member<bool>("hasOri")
        .member<bool>("hasAcc");
        //.set_doc_brief("Coordinate in a given system")
        //.set_doc_detailed("Coordinate in a given system, with position, velocity, orientation, and acceleration");



    logger->trace("Components Registered");



    ecs.system<simCore::Coordinate,
              const Pawn::Position,
              const Pawn::Velocity>("System_CoordinateUpdate")
        .with<Pawn::PawnOccupying>(flecs::Wildcard)
        .term_at(0).inout()
        .term_at(1).in()
        .term_at(2).in()
        .each([](flecs::entity pawn, simCore::Coordinate& coord, const Pawn::Position& pos, const Pawn::Velocity& vel){
            ZoneScopedN("System_CoordinateUpdate");
            flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
            const Map::GridCellStatic* loc = currentCell.get<Map::GridCellStatic>();  // TODO: Put it into the query
            float scale_m_per_cell = 20;
            logger->trace("loc: {}, {}; pos: {}, {} -> coord: {}, {}", loc->x, loc->y, pos.x, pos.y, scale_m_per_cell * (loc->y + pos.y), scale_m_per_cell * (loc->x + pos.x));
            coord.setPosition(scale_m_per_cell * (loc->y + pos.y), scale_m_per_cell * (loc->x + pos.x), 0);
            coord.setVelocity(scale_m_per_cell * vel.y, scale_m_per_cell * vel.x, 0);
        });



}


}