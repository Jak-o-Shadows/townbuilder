#include "pawn/module.hpp"

#include "map/module.hpp"
#include "coordinates/module.hpp"
#include "buildings/module.hpp"

#include <iostream>

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
    // The event itself is now the destination, which will be picked up systems
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

void PawnOccupationWoodcutter::react(const DropResources_Event& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("PawnOccupationWoodcutter::react(DropResources_Event) called for entity {}", std::string(e.path()));
    e.remove<Target>(flecs::Wildcard);
    flecs::entity dropoff_building = flecs::entity(control.context().ecs.lookup("::Buildings::components::buildingsParent::Granary1"));
    fsmLogger->debug("Dropoff building entity: {}", std::string(dropoff_building.path()));
    e.add<Target>(dropoff_building);
    control.changeTo<PawnWoodcutterStateReturning>();
}

void PawnWoodcutterStateReturning::react(const Arrived_Event& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("PawnWoodcutterStateReturning::react(Arrived_Event) called for entity {}", std::string(e.path()));

    // Dump the resources into the dropoff
    flecs::entity pawn = flecs::entity(control.context().ecs, control.context().id);
    //flecs::entity dropoff = pawn.second<Target>();
    // TODO: This properly
    Buildings::Resources& global_resources = control.context().ecs.get_mut<Buildings::Resources>();
    Buildings::Resources& pawn_resources = pawn.get_mut<Buildings::Resources>();
    global_resources.fish += pawn_resources.fish;
    global_resources.stone += pawn_resources.stone;
    global_resources.wood += pawn_resources.wood;
    pawn_resources.fish = 0;
    pawn_resources.stone = 0;
    pawn_resources.wood = 0;
    fsmLogger->trace("Pawn {} has returned to dropoff with resources. Global resources are now: fish {}, stone {}, wood {}",
        std::string(pawn.path()),
        global_resources.fish,
        global_resources.stone,
        global_resources.wood);

    e.remove<Destination_Event>();
    e.remove<Target>(flecs::Wildcard);
    control.changeTo<PawnWoodcutterStateWalkingTo>();
}


void PawnWoodcutterStateWalkingTo::react(const Arrived_Event& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    fsmLogger->debug("PawnWoodcutterStateWalkingTo::react(Arrived_Event) called for entity {}", std::string(e.path()));
    e.remove<Destination_Event>();
    control.changeTo<PawnWoodcutterStateChopping>();
}
    


}