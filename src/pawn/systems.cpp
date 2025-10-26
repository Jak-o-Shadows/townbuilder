#include "pawn/module.hpp"

#include "msgLogging/module.hpp"
#include "tracy_zones.hpp"
#include "ticks/module.hpp"
#include "coordinates/module.hpp"
#include "map/module.hpp"
#include "async_system.hpp"

#include <functional>

namespace Pawn{

std::shared_ptr<spdlog::logger> systemsLogger;
std::shared_ptr<spdlog::logger> fsmLogger;


std::tuple<Coordinates::CellVelocity> calculate_next_velocity(
    const Coordinates::Grid& current,
    const Coordinates::Cell& local,
    const Destination_Event& dest,
    const PawnAbilityTraits& ability){
        ZoneScopedN("calculate_next_velocity");
        // Calculate the velocity needed to go towards the destination
        float dx = (dest.target.x - current.x) + (dest.local.x - local.x)/2.0f;
        float dy = (dest.target.y - current.y) + (dest.local.y - local.y)/2.0f;
        // Clamp as per speed
        //  Remember that this is per second, as in movement it is scaled by delta time
        dx = std::clamp(dx, -ability.speed, ability.speed);
        dy = std::clamp(dy, -ability.speed, ability.speed);

        // Add a dummy sleep in to pretend this system takes time to run, as if it were actually pathfinding
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return {Coordinates::CellVelocity{dx, dy}};
    }


systems::systems(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<Statemachine::components>();
    ecs.import<Coordinates::components>();
    ecs.import<Ticks::module>();

    systemsLogger->trace("Other flecs modules imported");


    // The logger for the FSM is defined differently so header-only can access it.
    // TODO: Should I just put this in a different flecs module for consistency?
    // TODO: Should I make a function for this in msgLogging/module.hpp?
    const auto& sinks = ecs.get<Logging::LoggerSink>().sinks;
    fsmLogger = std::make_shared<spdlog::logger>(std::string(m.path()) + ".fsm", sinks.begin(), sinks.end());
    fsmLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(fsmLogger);

    ecs.observer<PawnFSMContainer>("Observer_PawnFsmContainer")
        .event(flecs::OnAdd)
        .each([](flecs::entity pawn, PawnFSMContainer&) {
            ZoneScopedN("Observer_PawnFsmContainer");
            systemsLogger->trace("Creating extra components for Pawn FSM {}", std::string(pawn.path()));
            // Set the utility for each
            //  TODO: This should only be done for utilitarian states, but haven't got utilitarian states in yet, so doing for all
            pawn.set<Statemachine::StateUtility, Pawn::Alive>({0});
            pawn.set<Statemachine::StateUtility, Pawn::Idle>({0});
            pawn.set<Statemachine::StateUtility, Pawn::Walking>({0});
            pawn.set<Statemachine::StateUtility, Pawn::Working>({0});
            pawn.set<Statemachine::StateUtility, Pawn::Fleeing>({0});
            pawn.set<Statemachine::StateUtility, Pawn::Combat>({0});
            pawn.set<Statemachine::StateUtility, Pawn::Dead>({0});
            pawn.set<Statemachine::StateUtility, Pawn::PawnOccupationUnemployed>({0});
            pawn.set<Statemachine::StateUtility, Pawn::PawnOccupationWoodcutter>({0});
            pawn.set<Statemachine::StateUtility, Pawn::PawnWoodcutterStateWalkingTo>({0});
            pawn.set<Statemachine::StateUtility, Pawn::PawnWoodcutterStateReturning>({0});
            pawn.set<Statemachine::StateUtility, Pawn::PawnWoodcutterStateChopping>({0});
            systemsLogger->trace("Created extra components for Pawn FSM {}", std::string(pawn.path()));
    });

   
    // Put systems in
    
    ecs.observer<const Coordinates::Grid>("Observer_PawnOccupying")
        .with<IsAPawn>()
        .term_at(0).in()
        .event(flecs::OnSet)
        .each([&ecs](flecs::entity pawn, const Coordinates::Grid& grid){
            ZoneScopedN("Observer_PawnOccupying");
            const Map::Grid& map = ecs.get<Map::Grid>();
            pawn.add<PawnOccupying>(flecs::entity(pawn.world(), map.get(grid.x, grid.y)));
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
            const std::vector<float> testPoints = {timing.timeInState_s, timing.culmulativeTimeInState_s};
            util.utility = Statemachine::utility_calc(curve, testPoints);
            // The longer we've been alive, the less useful it is to stay alive
            systemsLogger->trace("Pawn {} Alive utility: {}", std::string(e.path()), util.utility);
        });

    ecs.system<PawnFSMContainer>("System_PawnFSM_Update")
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::entity e, PawnFSMContainer& fsmc){
            ZoneScopedN("System_PawnFSM_Update");
            fsmc.machine->update();
        })
        .set_doc_brief("Update the Pawn FSM each pawn tick. This is required for utility theory");

    // Register the pathfinding system as an async system   
    std::function<std::tuple<Coordinates::CellVelocity>(const Coordinates::Grid&, const Coordinates::Cell&, const Destination_Event&, const PawnAbilityTraits&)> func = calculate_next_velocity;
    Async::create_async_system(ecs, "Pawn_CalculateNextVelocity")
        .query<const Coordinates::Grid&, const Coordinates::Cell&, const Destination_Event&, const PawnAbilityTraits&>()
        .work(func)
        .tick_source(Ticks::tick_pawn_behaviour)
        .build();
    

    // Emit Arrived_Events
    // TODO: This should be an observer. Do I need to only do the observer on the thing that changes, and use .with<> for the rest?
    ecs.system<const Coordinates::Grid,
                 const Coordinates::Cell,
                 const Destination_Event,
                 PawnFSMContainer>("System_GiveEventPawnArrived")
        .term_at(0).in()
        .term_at(1).in()
        .term_at(2).in()
        .each([](flecs::entity e,
            const Coordinates::Grid& grid,
            const Coordinates::Cell& local,
            const Destination_Event& dest,
            PawnFSMContainer& fsmc){
            ZoneScopedN("System_GiveEventPawnArrived");
            // If the pawn is at the destination, emit an Arrived_Event
            const float epsilon = 0.01f;
            if (grid.x == dest.target.x && grid.y == dest.target.y &&
                std::abs(local.x - dest.local.x) < epsilon && std::abs(local.y - dest.local.y) < epsilon) {
                systemsLogger->trace("Pawn {} has arrived at its destination", std::string(e.path()));
                // Notify the FSM that we've arrived
                //fsmc.machine->react(Arrived_Event{});
                // TODO: The react isn't working, so do it manually
                e.remove<Destination_Event>();
            }
        });








    systemsLogger->trace("Systems Registered");

};

}