#include <tracy/Tracy.hpp>

#include "coordinates.hpp"

#include "msgLogging.hpp"
#include "componentsPawn.hpp"

#include <format>
#include <iostream>

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

    // TODO: The hidden variables in the Coordinate struct need more work to be able to be used in the flecs explorer

    logger->trace("Components Registered");



    ecs.system<NED,
              const Pawn::Position,
              const Pawn::Velocity>("System_CoordinateUpdate")
        .with<Pawn::PawnOccupying>(flecs::Wildcard)
        .term_at(0).inout()
        .term_at(1).in()
        .term_at(2).in()
        .each([](flecs::entity pawn, NED& ned, const Pawn::Position& pos, const Pawn::Velocity& vel){
            ZoneScopedN("System_CoordinateUpdate");
            flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
            const Map::GridCellStatic* loc = currentCell.get<Map::GridCellStatic>();  // TODO: Put it into the query
            float scale_m_per_cell = 20;
            logger->trace("loc: {}, {}; pos: {}, {} -> ned: {}, {}", loc->x, loc->y, pos.x, pos.y, scale_m_per_cell * (loc->y + pos.y), scale_m_per_cell * (loc->x + pos.x));
            ned.setPosition(scale_m_per_cell * (loc->y + pos.y), scale_m_per_cell * (loc->x + pos.x), 0);
            ned.setVelocity(scale_m_per_cell * vel.y, scale_m_per_cell * vel.x, 0);
        });


    ecs.system<const NED, LLA, Converter>("System_NEDtoLLA")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<NedBase>()
        .each([](const NED& ned, LLA& lla, Converter& converter){
            ZoneScopedN("System_NEDtoLLA");
            converter.converter.convert(ned, lla, simCore::COORD_SYS_LLA);
        });

    ecs.system<const NED, ECEF, Converter>("System_NEDtoECEF")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<NedBase>()
        .each([](const NED& ned, ECEF& ecef, Converter& converter){
            ZoneScopedN("System_NEDtoECEF");
            converter.converter.convert(ned, ecef, simCore::COORD_SYS_ECEF);
        });
    
    ecs.system<const ECEF, NED, Converter>("System_ECEFtoNED")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<EcefBase>()
        .each([](const ECEF& ecef, NED& ned, Converter& converter){
            ZoneScopedN("System_ECEFtoNED");
            converter.converter.convert(ecef, ned, simCore::COORD_SYS_NED);
        });

    ecs.system<const ECEF, LLA, Converter>("System_ECEFtoLLA")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<EcefBase>()
        .each([](const ECEF& ecef, LLA& lla, Converter& converter){
            ZoneScopedN("System_ECEFtoLLA");
            converter.converter.convert(ecef, lla, simCore::COORD_SYS_LLA);
        });

    ecs.system<const LLA, NED, Converter>("System_LLAtoNED")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<LlaBase>()
        .each([](const LLA& lla, NED& ned, Converter& converter){
            ZoneScopedN("System_LLAtoNED");
            converter.converter.convert(lla, ned, simCore::COORD_SYS_NED);
        });

    ecs.system<const LLA, ECEF, Converter>("System_LLAtoECEF")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<LlaBase>()
        .each([](const LLA& lla, ECEF& ecef, Converter& converter){
            ZoneScopedN("System_LLAtoECEF");
            converter.converter.convert(lla, ecef, simCore::COORD_SYS_ECEF);
        });









    ecs.system<const NED, const LLA, const ECEF>("System_CoordPrint")
        .term_at(0).in()
        .term_at(1).in()
        .term_at(2).in()
        .interval(1)
        .each([](const NED& ned, const LLA& lla, const ECEF& ecef){
            ZoneScopedN("System_CoordPrint");
            std::string msg = std::format("NED: {}, {}, {}; LLA: {}, {}, {}; ECEF: {}, {}, {}", 
                ned.x(), ned.y(), ned.z(),
                lla.lat()*180/M_PI, lla.lon()*180/M_PI, lla.alt(),
                ecef.x(), ecef.y(), ecef.z());
            std::cout << msg << std::endl;
            logger->trace(msg);
        });


}


}