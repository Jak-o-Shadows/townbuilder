#include "pawn/module.hpp"

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"
#include "coordinates/module.hpp"
#include "map/module.hpp"

namespace Pawn{

std::shared_ptr<spdlog::logger> systemsLogger;
std::shared_ptr<spdlog::logger> fsmLogger;

systems::systems(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<Statemachine::components>();

    systemsLogger->trace("Other flecs modules imported");


    // The logger for the FSM is defined differently so header-only can access it.
    //  TODO: Should I just put this in a different flecs module for consistency?
    fsmLogger = std::make_shared<spdlog::logger>(std::string(m.path()) + ".fsm", ecs.get<Logging::LoggerSink>()->sink);
    fsmLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(fsmLogger);





   
    // Put systems in
    
    ecs.observer<const Coordinates::Grid>("Observer_PawnOccupying")
        .with<IsAPawn>()
        .term_at(0).in()
        .event(flecs::OnSet)
        .each([&ecs](flecs::entity pawn, const Coordinates::Grid& grid){
            ZoneScopedN("Observer_PawnOccupying");
            const Map::Grid* map = ecs.get<Map::Grid>();
            if (!map) {
                systemsLogger->error("Map not found when setting PawnOccupying");
                return;
            }
            pawn.add<PawnOccupying>(flecs::entity(pawn.world(), map->get(grid.x, grid.y)));
        });





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


    /*
    ecs.system<Pawn::PawnPathfindingGoal>("Pawn_Walk")
        //.with<LogicPawn::Walking>()//.or_().with<LogicPawn::PawnWoodcutterStateWalkingTo>().or_().with<LogicPawn::PawnWoodcutterStateReturning>()
        //.term_at(0).second("$goal")
        //.term_at(0).in()
        //.tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::entity e){
            ZoneScopedN("Pawn_Walk");
            //flecs::entity dest = e.target_for<Pawn::PawnPathfindingGoal>(flecs::ChildOf);
            //std::cout << e.path() << " : " << dest.path() << std::endl;
        });
    */
   
   ecs.system<Coordinates::Grid, Pawn::PawnPathfindingGoal>("Pawn_Walk")
        .term_at(0).in()
        .term_at(1).second("$goal")
        .term_at(1).in()
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::entity e, const Coordinates::Grid& grid, const Pawn::PawnPathfindingGoal& goal) {
            ZoneScopedN("Pawn_Walk");
            flecs::entity dest = e.target_for<Pawn::PawnPathfindingGoal>(flecs::ChildOf);
            systemsLogger->trace("Pawn {} walking to {}", std::string(e.path()), std::string(dest.path()));
        });
    
    ecs.system<Statemachine::StateUtility,
              const Statemachine::StateTiming,
              const Statemachine::Curve>("System_UtilityPawnAlive")
        .term_at(0).out()
        .term_at(0).second<Alive>()
        .term_at(1).in()
        .term_at(1).second<Alive>()
        .term_at(2).in()
        .term_at(2).second<Alive>()
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::entity e,
                 Statemachine::StateUtility& util,
                 const Statemachine::StateTiming& timing,
                 const Statemachine::Curve &curve){
            ZoneScopedN("System_UtilityPawnAlive");
            std::vector<float> testPoints = {timing.timeInState_s, timing.culmulativeTimeInState_s};
            util.utility = Statemachine::utility_calc(curve, testPoints);
            // The longer we've been alive, the less useful it is to stay alive
            systemsLogger->trace("Pawn {} Alive utility: {}", std::string(e.path()), util.utility);
        });

    ecs.system<PawnFSMContainer>("System_PawnFSM_Update")
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::entity e, PawnFSMContainer& fsmc){
            ZoneScopedN("System_PawnFSM_Update");
            fsmc.machine->update();
        });


        








    systemsLogger->trace("Systems Registered");

};

}