#include <tracy/Tracy.hpp>

#include "coordinates/module.hpp"

#include "msgLogging/module.hpp"
#include "pawn/module.hpp"

#include <format>
#include <iostream>

namespace Coordinates {

std::shared_ptr<spdlog::logger> componentsLogger;


components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Module Created");



    // Register components
    ecs.component<Grid>("Grid")
        .member<int>("x")
        .member<int>("y");
    
    ecs.component<Cell>("Cell")
        .member<float>("x")
        .member<float>("y")
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
    
    componentsLogger->trace("Components Registered");

};


}