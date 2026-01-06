#include "pawn/module.hpp"

#include "msgLogging/module.hpp"
#include "tracy_zones.hpp"
#include "ticks/module.hpp"
#include "coordinates/module.hpp"
#include "map/module.hpp"
#include "buildings/module.hpp"
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
        float dx = static_cast<float>(dest.target.x - current.x) + (dest.local.x - local.x)/2.0f;
        float dy = static_cast<float>(dest.target.y - current.y) + (dest.local.y - local.y)/2.0f;
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
    ecs.import<Map::components>();
    ecs.import<Buildings::components>();


    systemsLogger->trace("Other flecs modules imported");


    // The logger for the FSM is defined differently so header-only can access it.
    // TODO: Should I just put this in a different flecs module for consistency?
    // TODO: Should I make a function for this in msgLogging/module.hpp?
    const auto& sinks = ecs.get<Logging::LoggerSink>().sinks;
    fsmLogger = std::make_shared<spdlog::logger>(std::string(m.path()) + ".fsm", sinks.begin(), sinks.end());
    fsmLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(fsmLogger);


    
    ecs.observer<Pawn::IsAPawn>("Observer_AddPawnFSM")
        .event(flecs::OnAdd)
        .each([&ecs](flecs::entity e, const Pawn::IsAPawn&) {
            ZoneScopedN("Observer_AddPawnFSM");
            if (!e.has<Pawn::PawnFSMContainer>()) {
                systemsLogger->trace("Creating Pawn FSM for {}", std::string(e.path()));
                Statemachine::Context context{e.id(), ecs};
                e.set<Pawn::PawnFSMContainer>({std::shared_ptr<Pawn::PawnFSM::Instance>(new Pawn::PawnFSM::Instance(context))});
                systemsLogger->debug("Created Pawn FSM for {}", std::string(e.path()));
            }
        });


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
    
    /*
    ecs.observer<const Coordinates::Grid>("Observer_PawnOccupying")
        .with<IsAPawn>()
        .term_at(0).in()
        .event(flecs::OnSet)
        .each([&ecs](flecs::entity pawn, const Coordinates::Grid& grid){
            ZoneScopedN("Observer_PawnOccupying");
            const Map::Grid* map = ecs.try_get<Map::Grid>();  // TODO: This is incorrect, as the map:Grid is NOT a singleton, but is attached to an entity (for multipe level reasons?)
            if (!map) {
                systemsLogger->error("Cannot set PawnOccupying for {} as Map::Grid component not found", std::string(pawn.path()));
                return;
            }
            pawn.add<PawnOccupying>(flecs::entity(pawn.world(), map->get(grid.x, grid.y)));
        });
    */




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
        .each([](PawnFSMContainer& fsmc){
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
        .without<Pawn::Idle>()  // Only check pawns that aren't idle (as idle pawns won't be moving)
        .each([](flecs::entity e,
            const Coordinates::Grid& grid,
            const Coordinates::Cell& local,
            const Destination_Event& dest,
            PawnFSMContainer& fsmc){
            ZoneScopedN("System_GiveEventPawnArrived");
            // If the pawn is at the destination, emit an Arrived_Event
            const float epsilon = 0.1f;
            if (grid.x == dest.target.x && grid.y == dest.target.y &&
                std::abs(local.x - dest.local.x) < epsilon && std::abs(local.y - dest.local.y) < epsilon) {
                systemsLogger->debug("Pawn {} has arrived at its destination", std::string(e.path()));
                // Notify the FSM that we've arrived
                fsmc.machine->react(Arrived_Event{});
            }
        });


        
    ecs.observer<Target>("Observer_PawnTarget_Woodcutter")
        .event(flecs::OnAdd)
        .term_at(0).second(flecs::Wildcard)
        .with<PawnOccupationWoodcutter>()
        .with<PawnFSMContainer>()
        .each([](flecs::iter& it, size_t i, const Target&){
            ZoneScopedN("Observer_PawnTarget_Woodcutter");
            flecs::entity pawn = it.entity(i);
            systemsLogger->debug("Setting walking destination for Pawn {}", std::string(pawn.path()));
            flecs::entity tree = pawn.target<Target>();
            // Tell the pawn to walk to the tree
            PawnFSMContainer& fsmc = pawn.get_mut<PawnFSMContainer>();
            systemsLogger->trace("Sending Destination_Event to FSM on {}", std::string(pawn.path()));
            Destination_Event dest{tree.get<Coordinates::Grid>(), tree.get<Coordinates::Cell>()};
            systemsLogger->trace("Destination is: ({}, {}), ({}, {})",
                dest.target.x, dest.target.y,
                dest.local.x, dest.local.y);
            fsmc.machine->react(dest);
        });

    ecs.observer<Target>("Observer_PawnTargetRemoved_Woodcutter")
        .event(flecs::OnRemove)
        .term_at(0).second(flecs::Wildcard)
        .with<PawnWoodcutterStateChopping>()
        .with<PawnFSMContainer>()
        .each([&ecs](flecs::iter& it, size_t i, const Target&){
            ZoneScopedN("Observer_PawnTargetRemoved_Woodcutter");
            flecs::entity pawn = it.entity(i);
            systemsLogger->debug("Seeking new tree to cut for Pawn {}", std::string(pawn.path()));

            PawnFSMContainer& fsmc = pawn.get_mut<PawnFSMContainer>();
            fsmc.machine->changeTo<PawnWoodcutterStateWalkingTo>();
            fsmc.machine->update();

            // TODO: A LOT of redundant code with Woodcutter::enter. We also don't have the whole "return" part in

            // Now find the narest tree, and set it as the destination
            flecs::entity tree_prefab = ecs.lookup("::Map::Tree_Prefab");

            if (!tree_prefab) {
                systemsLogger->trace("Tree prefab not found - cannot tell pawn to go towards a tree");
                return;
            }

            flecs::entity nearest = Coordinates::find_nearest_by_grid(ecs, pawn, tree_prefab);
            if (nearest){
                systemsLogger->trace("Nearest {} to {} is {}",
                    std::string(tree_prefab.path()),
                    std::string(pawn.path()),
                    std::string(nearest.path()));
                // Set the tree as the "target". This will then trigger the destination event
                pawn.add<Target>(nearest);
            } else {
                systemsLogger->trace("Cannot find any trees");
            }
        });
        
    
    ecs.system<const PawnAbilityTraits, const Coordinates::Grid>("System_PawnWoodcut")
        .term_at(0).in()
        .term_at(1).in()
        .with<Target>().second(flecs::Wildcard)
        .with<PawnWoodcutterStateChopping>()
        .tick_source(Ticks::tick_pawn_behaviour)
        .each([](flecs::iter& it, size_t i,
            const PawnAbilityTraits& ability,
            const Coordinates::Grid& grid){
                ZoneScopedN("System_PawnWoodcut");
                flecs::entity pawn = it.entity(0);

                // Get the second of the target. This should be the tree we're at
                flecs::entity tree = pawn.target<Target>();
                if (!tree){
                    systemsLogger->error("Pawn {} has no Pawn::Target for woodcutting", std::string(pawn.path()));
                    return;
                }

                float damage = ability.woodcut_speed * it.delta_system_time();

                const Coordinates::Grid& tree_grid = tree.get<Coordinates::Grid>();
                if (tree_grid.x == grid.x && tree_grid.y == grid.y) {
                    Buildings::Resources& resources = tree.get_mut<Buildings::Resources>();
                    resources.wood -= damage;
                }
            });

    








    systemsLogger->trace("Systems Registered");

};

}