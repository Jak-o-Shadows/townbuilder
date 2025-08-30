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

    //  Each cell of the map is an entity
    // Stored in a vector for each access
    const Map::Grid* map = ecs.get<Map::Grid>();
   
    flecs::id_t id2 = map->get(dest.x, dest.y);
    flecs::entity e2 = flecs::entity(ecs, id2);
    e.add<Pawn::PawnPathfindingGoal>(e2);
    fsmLogger->trace("Walking::react(Destination_Event) called for entity {} with destination ({}, {}): {}", std::string(e.path()), dest.x, dest.y, std::string(e2.path()));
}

void Walking::react(const Arrived_Event&, FullControl& control){
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    flecs::world& ecs = control.context().ecs;

    // We arrived -> remove the destination
    e.remove<Pawn::PawnPathfindingGoal>();

    control.changeTo<Idle>();
}




}