#include "pawn/module.hpp"

#include "map/module.hpp"

namespace Pawn{

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

void Walking::react(const Destination_Event& dest, FullControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    flecs::world& ecs = control.context().ecs;
    // The event itself is now the destination, which will be picked up by the
    // OnEnterWalkingState_RequestPath observer.
    e.set<Destination_Event>(dest);
    fsmLogger->trace("Walking::react(Destination_Event) called for entity {} with destination ({}, {})", std::string(e.path()), dest.x, dest.y);
}

void Walking::react(const Arrived_Event&, FullControl& control){
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);

    // We arrived -> remove the destination
    e.remove<Pawn::PathfindRequest>();
    e.remove<Destination_Event>();

    control.changeTo<Idle>();
}




}