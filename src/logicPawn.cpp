#include "logicPawn2.hpp"

#include <tracy/Tracy.hpp>

#include "gridMap.hpp"
#include "componentsPawn.hpp"


namespace LogicPawn {

module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();


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


};

// top-level region in the hierarchy
void Alive::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    char msg[100];
    sprintf(&msg[0], "%s(%i) -> Alive", e.name().c_str(), (int) e.id());
    TracyMessage(msg, 100);
    e.add<Alive>();
}
void Alive::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Alive>();
}
//------------------------------------------------------------------------------

void Idle::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.add<Idle>();
}
void Idle::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Idle>();
}

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
void Working::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    char msg[100];
    sprintf(&msg[0], "%s(%i) -> Working", e.name().c_str(), (int) e.id());
    TracyMessage(msg, 100);
    e.add<Working>();
}
void Working::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Working>();
}

//void Working::update(FullControl& control) {
//    control.changeTo<Fleeing>();
//}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Walking::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    char msg[100];
    sprintf(&msg[0], "%s(%i) -> Walking", e.name().c_str(), (int) e.id());
    TracyMessage(msg, 100);
    e.add<Walking>();
}

void Walking::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Walking>();
}

void Walking::react(const Arrived_Event&, FullControl& control){
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    flecs::world& ecs = control.context().ecs;
    // Randomly generate a new place
    int targetX = 5;
    int targetY = 5;

    //  Each cell of the map is an entity

    // Stored in a vector for each access
    std::shared_ptr<Map::grid> map = Map::mapEntity.get<Map::MapContainer>()->map;
    int map_width = map->m_width;
    int map_height = map->m_height;

    e.add<Pawn::PawnPathfindingGoal>(flecs::entity(ecs, map->get(targetX, targetY)));
    std::cout << e.name() << "(" << e.id() << ")" << " Arrived" << std::endl;
    control.changeTo<Walking>();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Combat::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.add<Combat>();
}
void Combat::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Combat>();
}

void Combat::update(FullControl& control) {
    control.changeTo<Idle>();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Fleeing::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.add<Fleeing>();
}
void Fleeing::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Fleeing>();
}

void Fleeing::update(FullControl& control) {
    control.changeTo<Idle>();
}

//------------------------------------------------------------------------------

// another top-level state

void Dead::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.add<Dead>();
}
void Dead::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Dead>();
}




}