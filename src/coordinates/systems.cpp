#include "coordinates/module.hpp"

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"

#include <tracy/Tracy.hpp>

#include <math.h>
#include <iostream>

namespace Coordinates {

std::shared_ptr<spdlog::logger> systemsLogger;


systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<Ticks::module>();
    
    ecs.system<NED,
              const Grid,
              const Cell,
              const CellVelocity>("System_GridToNed")
        .term_at(0).inout()
        .term_at(1).in()
        .term_at(2).in()
        .term_at(3).in()
        .with<GridBase>()
        .each([](flecs::entity pawn, NED& ned, const Grid& grid, const Cell& cell, const CellVelocity& vel){
            ZoneScopedN("System_GridToNed");
            float scale_m_per_cell = 20;
            systemsLogger->trace("loc: {}, {}; pos: {}, {} -> ned: {}, {}", grid.x, grid.y, cell.x, cell.y, scale_m_per_cell * (grid.y + cell.y), scale_m_per_cell * (grid.x + cell.x));
            ned.setPosition(scale_m_per_cell * (grid.y + cell.y), scale_m_per_cell * (grid.x + cell.x), 0);
            ned.setVelocity(scale_m_per_cell * vel.y, scale_m_per_cell * vel.x, 0);
            systemsLogger->trace("ned: {}, {}, {} m, {}, {}, {} m/s", ned.x(), ned.y(), ned.z(), ned.vx(), ned.vy(), ned.vz());
        })
        .set_doc_brief("Set NED coordinates based on Grid and Cell coordinates, and CellVelocity");


    ecs.system<const NED, LLA, Converter>("System_NEDtoLLA")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<NedBase>().or_().with<GridBase>()
        .each([](const NED& ned, LLA& lla, Converter& converter){
            ZoneScopedN("System_NEDtoLLA");
            converter.converter.convert(ned, lla, simCore::COORD_SYS_LLA);
        })
        .set_doc_brief("Convert NED coordinates to LLA coordinates");

    ecs.system<const NED, ECEF, Converter>("System_NEDtoECEF")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<NedBase>().or_().with<GridBase>()
        .each([](const NED& ned, ECEF& ecef, Converter& converter){
            ZoneScopedN("System_NEDtoECEF");
            converter.converter.convert(ned, ecef, simCore::COORD_SYS_ECEF);
        })
        .set_doc_brief("Convert NED coordinates to ECEF coordinates");
    
    ecs.system<const ECEF, NED, Converter>("System_ECEFtoNED")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<EcefBase>()
        .each([](const ECEF& ecef, NED& ned, Converter& converter){
            ZoneScopedN("System_ECEFtoNED");
            converter.converter.convert(ecef, ned, simCore::COORD_SYS_NED);
        })
        .set_doc_brief("Convert ECEF coordinates to NED coordinates");

    ecs.system<const ECEF, LLA, Converter>("System_ECEFtoLLA")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<EcefBase>()
        .each([](const ECEF& ecef, LLA& lla, Converter& converter){
            ZoneScopedN("System_ECEFtoLLA");
            converter.converter.convert(ecef, lla, simCore::COORD_SYS_LLA);
        })
        .set_doc_brief("Convert ECEF coordinates to LLA coordinates");

    ecs.system<const LLA, NED, Converter>("System_LLAtoNED")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<LlaBase>()
        .each([](const LLA& lla, NED& ned, Converter& converter){
            ZoneScopedN("System_LLAtoNED");
            converter.converter.convert(lla, ned, simCore::COORD_SYS_NED);
        })
        .set_doc_brief("Convert LLA coordinates to NED coordinates");

    ecs.system<const LLA, ECEF, Converter>("System_LLAtoECEF")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).singleton()
        .with<LlaBase>()
        .each([](const LLA& lla, ECEF& ecef, Converter& converter){
            ZoneScopedN("System_LLAtoECEF");
            converter.converter.convert(lla, ecef, simCore::COORD_SYS_ECEF);
        })
        .set_doc_brief("Convert LLA coordinates to ECEF coordinates");

    // Defined after the other converts so that it runs after them
    ecs.system<const NED, Grid, Cell, CellVelocity>("System_NEDtoGrid")
        .term_at(0).in()
        .term_at(1).out()
        .term_at(2).out()
        .term_at(3).out()
        .without<GridBase>()
        .each([](const NED& ned, Grid& grid, Cell& cell, CellVelocity& vel){
            ZoneScopedN("System_NEDtoGrid");
            float scale_m_per_cell = 20;
            grid.x = static_cast<int>(ned.x() / scale_m_per_cell);
            grid.y = static_cast<int>(ned.y() / scale_m_per_cell);
            cell.x = static_cast<int>((ned.x() - grid.x * scale_m_per_cell) / scale_m_per_cell * 2 - 1);
            cell.y = static_cast<int>((ned.y() - grid.y * scale_m_per_cell) / scale_m_per_cell * 2 - 1);
            vel.x = ned.vx();
            vel.y = ned.vy();
        })
        .set_doc_brief("Set Grid and Cell coordinates based on NED coordinates");


    // Add observers that add the other coordinate systems depending on the base system
    ecs.observer<const GridBase>("Observer_AddCoordinates_GridBase")
        .event(flecs::OnAdd)
        .each([](flecs::entity e, const GridBase&){
            ZoneScopedN("Observer_AddCoordinates_GridBase");
            e.add<Coordinates::NED>();
            e.add<Coordinates::LLA>();
            e.add<Coordinates::ECEF>();
        });

    ecs.observer<const NedBase>("Observer_AddCoordinates_NedBase")
        .event(flecs::OnAdd)
        .each([](flecs::entity e, const NedBase&){
            ZoneScopedN("Observer_AddCoordinates_NedBase");
            e.add<Coordinates::LLA>();
            e.add<Coordinates::ECEF>();
            e.add<Coordinates::Grid>();
            e.add<Coordinates::Cell>();
            e.add<Coordinates::CellVelocity>();
        });
        
    ecs.observer<const LlaBase>("Observer_AddCoordinates_LlaBase")
    .event(flecs::OnAdd)
    .each([](flecs::entity e, const LlaBase&){
        ZoneScopedN("Observer_AddCoordinates_LlaBase");
        e.add<Coordinates::NED>();
        e.add<Coordinates::ECEF>();
        e.add<Coordinates::Grid>();
        e.add<Coordinates::Cell>();
        e.add<Coordinates::CellVelocity>();
    });

    ecs.observer<const EcefBase>("Observer_AddCoordinates_EcefBase")
        .event(flecs::OnAdd)
        .each([](flecs::entity e, const EcefBase&){
            ZoneScopedN("Observer_AddCoordinates_EcefBase");
            e.add<Coordinates::NED>();
            e.add<Coordinates::LLA>();
            e.add<Coordinates::Grid>();
            e.add<Coordinates::Cell>();
            e.add<Coordinates::CellVelocity>();
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
            systemsLogger->debug(msg);
        });





    auto move_sys = ecs.system<Cell, const CellVelocity>("System_IntraGridMovement")
    .term_at(0).inout()
    .term_at(1).in()
    .with<GridBase>()
    .tick_source(Ticks::tick_pawn_behaviour)
    .each([](flecs::iter& it, size_t i, Cell& p, const CellVelocity& v){
        ZoneScopedN("System_IntraGridMovement");
        p.x += v.x * it.delta_system_time();
        p.y += v.y * it.delta_system_time();
        systemsLogger->trace("Position: {}, {} @ Velocity: {}, {}", p.x, p.y, v.x, v.y);
    })
    .set_doc_brief("Update Cell position based on CellVelocity");

    ecs.system<Cell, Grid>("System_InterGridMovement")
        .term_at(0).inout()
        .term_at(1).inout()
        .with<GridBase>()
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::entity e, Cell& cell, Grid& grid){
            ZoneScopedN("System_InterGridMovement");
            bool changed = false;
            if (cell.x < -1) {
                cell.x += 2;
                grid.x -= 1;
                changed = true;
            }
            else if (cell.x > 1) {
                cell.x -= 2;
                grid.x += 1;
                changed = true;
            }
            if (cell.y < -1) {
                cell.y += 2;
                grid.y -= 1;
                changed = true;
            }
            else if (cell.y > 1) {
                cell.y -= 2;
                grid.y += 1;
                changed = true;
            }
            if (changed) {
                systemsLogger->trace("Entity {} moved to new grid {}, {}", std::string(e.path()), grid.x, grid.y);
            }
        })
        .set_doc_brief("Update Grid position based on Cell position, if outside of [-1, 1] range");






    systemsLogger->trace("Systems Registered");








};

}