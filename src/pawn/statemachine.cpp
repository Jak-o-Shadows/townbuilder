#include "pawn/module.hpp"

#include "map/module.hpp"

namespace Pawn{

//------------------------------------------------------------------------------

void Alive::react(const Attacked& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("Alive::react(Attacked) called for entity {}", std::string(e.path()));
    control.changeTo<Combat>();
}

void Alive::react(const Arrived_Event& event, EventControl& control){
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("Alive::react(Arrived_Event) called for entity {}", std::string(e.path()));
    //e.remove<Destination_Event>();
    //control.changeTo<Idle>();
}



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
/*
void Idle::react(const Destination_Event& dest, FullControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("Idle::react(Destination_Event) called for entity {} with destination ({}, {})", std::string(e.path()), dest.target.x, dest.target.y);
    // The event itself is now the destination, which will be picked up by the
    // OnEnterWalkingState_RequestPath observer.
    std::cout << "Idle::React(Destination_Event)" << std::endl;
    e.set<Destination_Event>(dest);
    control.changeTo<Walking>();
}
    */


void Walking::react(const Arrived_Event& event, EventControl& control){
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("Walking::react(Arrived_Event) called for entity {}", std::string(e.path()));
    e.remove<Destination_Event>();
    control.changeTo<Idle>();
}

void Walking::react(const Destination_Event& dest, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("Walking::react(Destination_Event) called for entity {} with destination ({}, {})", std::string(e.path()), dest.target.x, dest.target.y);
    // The event itself is now the destination, which will be picked up by the
    // OnEnterWalkingState_RequestPath observer.
    e.set<Destination_Event>(dest);
}

/*
void PawnWoodcutterStateWalkingTo::react(const Arrived_Event&, FullControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("PawnWoodcutterStateWalkingTo::react(Arrived_Event) called for entity {}", std::string(e.path()));
    control.changeTo<PawnWoodcutterStateChopping>();
}
*/
    


}