#include "logicPawn2.hpp"

namespace LogicPawn {

module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();


};

// top-level region in the hierarchy
void Alive::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
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
void Idle::update(FullControl& control) {
    // multiple transitions can be initiated, can be useful in a hierarchy
    //if (control.context().cycleCount > 3)
    //	control.changeTo<Off>();
    //else
        control.changeTo<Working>();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Working::enter(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.add<Working>();
}
void Working::exit(Control& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    e.remove<Working>();
}

void Working::update(FullControl& control) {
    control.changeTo<Fleeing>();
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