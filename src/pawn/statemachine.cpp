#include "pawn/module.hpp"

#include "map/module.hpp"
#include "coordinates/module.hpp"

namespace Pawn{

//------------------------------------------------------------------------------

void Alive::react(const Attacked& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("Alive::react(Attacked) called for entity {}", std::string(e.path()));
    control.changeTo<Combat>();
}

//void Alive::react(const Arrived_Event& event, EventControl& control){
//    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
//    fsmLogger->debug("Alive::react(Arrived_Event) called for entity {}", std::string(e.path()));
    //e.remove<Destination_Event>();
    //control.changeTo<Idle>();
//}



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


void PawnOccupationWoodcutter::enter(Control& control) {
    // Call the base class enter
    BasePawnState<PawnOccupationWoodcutter>::enter(control);

}

void PawnOccupationWoodcutter::react(const Destination_Event& dest, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("PawnOccupationWoodcutter::react(Destination_Event) called for entity {} with destination ({}, {})", std::string(e.path()), dest.target.x, dest.target.y);
    // The event itself is now the destination, which will be picked up by the
    // OnEnterWalkingState_RequestPath observer.
    e.set<Destination_Event>(dest);
}




void PawnWoodcutterStateWalkingTo::react(const Arrived_Event& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("PawnWoodcutterStateWalkingTo::react(Arrived_Event) called for entity {}", std::string(e.path()));

    /*
    // Must set velocity to zero, otherwise pawn will keep moving.
    e.set<Coordinates::CellVelocity>({0, 0});  
    // Also remove the future calculation, if it exists, otherwise celLVelocity will get set again
    flecs::entity future = control.context().ecs.lookup("Pawn_CalculateNextVelocity_Future");
    e.remove(future);
    fsmLogger->trace("Zeroed velocity for pawn {}", std::string(e.path()));

    // By definition, if we arrived we are athe right location. Set the location to exactly match the destination,
    //  as otherwise we might have some floating point error that causes us to never actually arrive.
    //  Get the location from the Destination_Event, which should still be present on the entity, and set the Cell to match it.
    const Destination_Event* dest = e.try_get<Destination_Event>();
    if(dest){
        e.set<Coordinates::Cell>({dest->local.x, dest->local.y});
        e.set<Coordinates::Grid>({dest->target.x, dest->target.y});
        fsmLogger->trace("Set cell for pawn {} to ({}, {})", std::string(e.path()), dest->target.x, dest->target.y);
    } else {
        fsmLogger->warn("Pawn {} arrived at destination but no Destination_Event found!", std::string(e.path()));
    }
    */

    e.remove<Destination_Event>();

    control.changeTo<PawnWoodcutterStateChopping>();

}
    


}