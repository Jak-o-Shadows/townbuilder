#include "logicPawn2.hpp"

#include <tracy/Tracy.hpp>

#include "gridMap.hpp"
#include "componentsPawn.hpp"
#include "ticks.hpp"


namespace LogicPawn {

std::shared_ptr<spdlog::logger> logger;

module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using loggers, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});

    // Register components
    ecs.component<StateTiming>()
        .member<float>("timeInState_s")
        .member<float>("culmulativeTimeInState_s");


    // State actions
    /*
    auto blah_sys = ecs.system<>("ASDF")
        .with<PawnWoodcutterState>(ecs.component<PawnWoodcutterStateIdle>())
        .tick_source(tick_pawn_behaviour)
        .multi_threaded()
        .iter([](flecs::iter it){
            ZoneScopedN("Pawn Woodctuter Idle State Actions");
            for (int i: it){
                flecs::entity e = it.entity(i);
                // If they are idle, get them to find the nearest wood and path-find towards it
                
            }
    });
    */
    
    ecs.system<StateTiming>("Increment_StateTiming")
        .term_at(0).second("$state")
        .with("$state")
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](StateTiming& timing) {
            ZoneScopedN("Increment_StateTiming");
            float dt = 0.01;//it.delta_system_time();  // TODO: This needs to be the it.delta_system_time(), but not working
            timing.timeInState_s += dt;
            timing.culmulativeTimeInState_s += dt;
        });



    logger->trace("Module Created");

};



//------------------------------------------------------------------------------

// top-level region in the hierarchy




// state can initiate transitions to _any_ other state
/*
void Idle::update(FullControl& control) {
    // multiple transitions can be initiated, can be useful in a hierarchy
    //if (control.context().cycleCount > 3)
    //	control.changeTo<Off>();
    //else
        control.changeTo<Working>();
}
*/


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Walking::react(const Arrived_Event&, FullControl& control){
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    flecs::world& ecs = control.context().ecs;
    // Randomly generate a new place
    int targetX = 5;
    int targetY = 5;

    //  Each cell of the map is an entity

    // Stored in a vector for each access
    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();
    int map_width = map->m_width;
    int map_height = map->m_height;

    
    flecs::id_t id2 = map->get(targetX, targetY);
    flecs::entity e2 = flecs::entity(ecs, id2);
    //e.add<Pawn::PawnPathfindingGoal>(flecs::entity(ecs, map->get(targetX, targetY)));
    e.add<Pawn::PawnPathfindingGoal>(e2);
    std::cout << e.name() << "(" << e.id() << ")" << " Arrived" << std::endl;
    control.changeTo<Walking>();
    
}




}