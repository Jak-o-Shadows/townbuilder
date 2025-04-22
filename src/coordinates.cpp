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



    // Register components
    ecs.component<Grid>("Grid")
        .member<int>("x")
        .member<int>("y");
    ecs.component<Cell>("Cell")
        .member<int>("x")
        .member<int>("y")
        .set_doc_brief("Location within the grid cell. Limited to [-1, 1]");
    ecs.component<CellVelocity>("CellVelocity")
        .member<float>("x")
        .member<float>("y");

    // Register Simdis components
    ecs.component<simCore::Vec3>()
        .member<double>("x")
        .member<double>("y")
        .member<double>("z");

    ecs.component<simCore::CoordinateSystem>()
        .constant("COORD_SYS_NONE",  simCore::CoordinateSystem::COORD_SYS_NONE)
        .constant("COORD_SYS_NED",   simCore::CoordinateSystem::COORD_SYS_NED)
        .constant("COORD_SYS_NWU",   simCore::CoordinateSystem::COORD_SYS_NWU)
        .constant("COORD_SYS_ENU",   simCore::CoordinateSystem::COORD_SYS_ENU)
        .constant("COORD_SYS_LLA",   simCore::CoordinateSystem::COORD_SYS_LLA)
        .constant("COORD_SYS_ECEF",  simCore::CoordinateSystem::COORD_SYS_ECEF)
        .constant("COORD_SYS_ECI",   simCore::CoordinateSystem::COORD_SYS_ECI)
        .constant("COORD_SYS_XEAST", simCore::CoordinateSystem::COORD_SYS_XEAST)
        .constant("COORD_SYS_GTP",   simCore::CoordinateSystem::COORD_SYS_GTP)
        .constant("COORD_SYS_MAX",   simCore::CoordinateSystem::COORD_SYS_MAX);

    // TODO: The hidden variables in the Coordinate struct need more work to be able to be used in the flecs explorer
    //  they are somewhat visible now, but cause crashes? And why doesn't the enum work?
    ecs.component<simCore::Coordinate>()
        //.member<simCore::CoordinateSystem>("system_")
        .member<simCore::Vec3>("pos_")
        .member<simCore::Vec3>("vel_")
        .member<simCore::Vec3>("ori_")
        .member<simCore::Vec3>("acc_")
        .member<double>("elapsedEciTime_")
        .member<bool>("hasVel_")
        .member<bool>("hasOri_")
        .member<bool>("hasAcc_");
    ecs.component<LLA>()
        //.member<simCore::CoordinateSystem>("system_")
        .member<simCore::Vec3>("pos_")
        .member<simCore::Vec3>("vel_")
        .member<simCore::Vec3>("ori_")
        .member<simCore::Vec3>("acc_")
        .member<double>("elapsedEciTime_")
        .member<bool>("hasVel_")
        .member<bool>("hasOri_")
        .member<bool>("hasAcc_");
    ecs.component<NED>()
        //.member<simCore::CoordinateSystem>("system_")
        .member<simCore::Vec3>("pos_")
        .member<simCore::Vec3>("vel_")
        .member<simCore::Vec3>("ori_")
        .member<simCore::Vec3>("acc_")
        .member<double>("elapsedEciTime_")
        .member<bool>("hasVel_")
        .member<bool>("hasOri_")
        .member<bool>("hasAcc_");
    ecs.component<ECEF>()
        //.member<simCore::CoordinateSystem>("system_")
        .member<simCore::Vec3>("pos_")
        .member<simCore::Vec3>("vel_")
        .member<simCore::Vec3>("ori_")
        .member<simCore::Vec3>("acc_")
        .member<double>("elapsedEciTime_")
        .member<bool>("hasVel_")
        .member<bool>("hasOri_")
        .member<bool>("hasAcc_");

    logger->trace("Components Registered");



    ecs.system<NED,
              const Grid,
              const Cell,
              const CellVelocity>("System_CoordinateUpdate")
        .with<Pawn::PawnOccupying>(flecs::Wildcard)
        .term_at(0).inout()
        .term_at(1).in()
        .term_at(2).in()
        .term_at(3).in()
        .each([](flecs::entity pawn, NED& ned, const Grid& grid, const Cell& cell, const CellVelocity& vel){
            ZoneScopedN("System_CoordinateUpdate");
            float scale_m_per_cell = 20;
            logger->trace("loc: {}, {}; pos: {}, {} -> ned: {}, {}", grid.x, grid.y, cell.x, cell.y, scale_m_per_cell * (grid.y + cell.y), scale_m_per_cell * (grid.x + cell.x));
            ned.setPosition(scale_m_per_cell * (grid.y + cell.y), scale_m_per_cell * (grid.x + cell.x), 0);
            ned.setVelocity(scale_m_per_cell * vel.y, scale_m_per_cell * vel.x, 0);
            logger->trace("ned: {}, {}, {} m, {}, {}, {} m/s", ned.x(), ned.y(), ned.z(), ned.vx(), ned.vy(), ned.vz());
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
        .each([](flecs::entity e, const NED& ned, const LLA& lla, const ECEF& ecef){
            ZoneScopedN("System_CoordPrint");
            std::string msg = std::format("{}: NED: {}, {}, {}; LLA: {}, {}, {}; ECEF: {}, {}, {}", 
                e.path().c_str(),
                ned.x(), ned.y(), ned.z(),
                lla.lat()*180/M_PI, lla.lon()*180/M_PI, lla.alt(),
                ecef.x(), ecef.y(), ecef.z());
            logger->debug(msg);
        });


}


}