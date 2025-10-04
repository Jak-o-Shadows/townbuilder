#include "pawn/module.hpp"

#include "ticks/module.hpp"
#include "coordinates/module.hpp"

#include <random>

namespace Pawn {

// Handle extern entities
flecs::entity pawnsParent;
std::shared_ptr<spdlog::logger> componentsLogger;


components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Module Created");
    
    //ecs.import<Ticks::module>();  // TODO: Need to more registerModule out
    //ecs.import<Map::module>();

    
    // Modify pathfinding components
    // Each pawn can only occupy a single cell, so make exclusive
    ecs.component<PawnOccupying>().add(flecs::Exclusive);
    // Each pawn can only have a single next cell, so make exclusive
    ecs.component<PawnNextCell>().add(flecs::Exclusive);
    
    // Pathfinding components
    ecs.component<PathfindRequest>().add(flecs::Exclusive);
    ecs.component<Path>();


   
    // Register components with reflection data & documentation
    ecs.component<PawnLifeTraits>()
        .member<float>("hunger")
        .member<float>("thirst")
        .member<float>("cold")
        .member<float>("comfort");
    ecs.component<PawnAbilityTraits>()
        .member<float>("strength")
        .member<float>("speed");
    componentsLogger->trace("Components Registered");
    
    // Need to give the entities a parent so they show nicer in the flecs explorer
    pawnsParent = ecs.entity("pawns");




    componentsLogger->trace("Module Setup Complete");







};



}