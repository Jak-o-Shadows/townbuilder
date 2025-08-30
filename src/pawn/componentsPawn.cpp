#include "componentsPawn.hpp"
#include "ticks.hpp"
#include "coordinates.hpp"

#include <random>

namespace Pawn {

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
    
    //ecs.import<Ticks::module>();  // TODO: Need to more registerModule out
    //ecs.import<Map::module>();

    
    // Modify pathfinding components
    // Each pawn can only occupy a single cell, so make exclusive
    ecs.component<PawnOccupying>().add(flecs::Exclusive);
    // Each pawn can only have a single target cell, so make exclusive
    ecs.component<PawnPathfindingGoal>().add(flecs::Exclusive);
    // Each pawn can only have a single next cell, so make exclusive
    ecs.component<PawnNextCell>().add(flecs::Exclusive);
    

   
    // Register components with reflection data & documentation
    ecs.component<PawnLifeTraits>()
        .member<float>("hunger")
        .member<float>("thirst")
        .member<float>("cold")
        .member<float>("comfort");
    ecs.component<PawnAbilityTraits>()
        .member<float>("strength")
        .member<float>("speed");
    logger->trace("Components Registered");
    
    // Need to give the entities a parent so they show nicer in the flecs explorer
    pawnsParent = ecs.entity("pawns");



   
    // Put systems in
    auto move_sys = ecs.system<Coordinates::Cell, Coordinates::CellVelocity>("System_IntraCellMovement")
    .tick_source(Ticks::tick_pawn_behaviour)
    .run([](flecs::iter& it){
        ZoneScopedN("System_IntraCellMovement");
        while (it.next()){
            auto p = it.field<Coordinates::Cell>(0);
            auto v = it.field<Coordinates::CellVelocity>(1);
            for (auto i: it){
                p[i].x += v[i].x * it.delta_system_time();
                p[i].y += v[i].y * it.delta_system_time();
                logger->trace("Position: {}, {} @ Velocity: {}, {}", p[i].x, p[i].y, v[i].x, v[i].y);
            }
        }
    });
    
    ecs.observer<const Coordinates::Grid>("Observer_PawnOccupying")
        .with<IsAPawn>()
        .term_at(0).in()
        .event(flecs::OnSet)
        .each([&ecs](flecs::entity pawn, const Coordinates::Grid& grid){
            ZoneScopedN("Observer_PawnOccupying");
            const Map::Grid* map = ecs.get<Map::Grid>();
            if (!map) {
                logger->error("Map not found when setting PawnOccupying");
                return;
            }
            pawn.add<PawnOccupying>(flecs::entity(pawn.world(), map->get(grid.x, grid.y)));
        });

    logger->trace("Module Setup Complete");







};



}