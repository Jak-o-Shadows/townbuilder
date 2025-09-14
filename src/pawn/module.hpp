#pragma once

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "tracy_zones.hpp"

// optional: enable FSM structure report in debugger
#define HFSM2_ENABLE_ALL
#include <hfsm2/machine.hpp>

#include <iostream>
#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

#include "msgLogging/module.hpp"
#include "statemachine/module.hpp"

namespace Pawn{

struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

extern flecs::entity pawnsParent;

struct Pawn_Prefab {};
struct IsAPawn {};

struct PawnLifeTraits {
    float hunger;
    float thirst;
    float cold;
    float comfort;
};

struct PawnAbilityTraits {
    float strength;
    float speed;
};

struct PawnPathfindingGoal {};

struct PawnOccupying {};

struct PawnNextCell {};

struct Likes { };


extern std::shared_ptr<spdlog::logger> fsmLogger;







// data shared between FSM states and outside code

// convenience typedef
using M = hfsm2::MachineT<hfsm2::Config::ContextT<Statemachine::Context>>;


// Events
struct Destination_Event{
    int x;
    int y;
};
struct Arrived_Event {};  // When you arrive at a location or cell
struct SecondaryEvent { int payload; };
struct Attacked {};



// states need to be forward declared to be used in FSM struct declaration.
//    This also allows them to be used as tags in flecs
struct Alive;
struct Idle;
struct Walking;
struct Working;
struct Fleeing;
struct Combat;
struct Dead;

// Job & Job Related

// Pawn Occupations
struct PawnOccupationUnemployed;
struct PawnOccupationWoodcutter;


// Pawn States
//  Woodcutter
struct PawnWoodcutterStateWalkingTo;
struct PawnWoodcutterStateReturning;
struct PawnWoodcutterStateChopping;



using PawnFSM = M::PeerRoot<
                // sub-machine ..
                M::Utilitarian<Alive,
                    // .. with 4 sub-states
                    Idle,
                    M::Composite<Working,
                        PawnOccupationUnemployed,
                        M::Utilitarian<PawnOccupationWoodcutter,
                            PawnWoodcutterStateWalkingTo,
                            PawnWoodcutterStateReturning,
                            PawnWoodcutterStateChopping
                            >
                        >,
                    Walking,
                    Fleeing,
                    Combat
                >,
                Dead
            >;


template <typename TemplateState>
struct BasePawnState : PawnFSM::State {
    // BasePawnState is a base class for all Pawn states, providing default reactions
    void react(const Destination_Event&, FullControl& control) {};
    void react(const Arrived_Event&, FullControl& control) {};
    void react(const SecondaryEvent&, FullControl& control) {};
    void react(const Attacked&, FullControl& control) {};
    void enter(Control& control) {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        fsmLogger->trace("Pawn {} entering state {}", std::string(e.path()), Statemachine::TypeName<TemplateState>());
          // TODO: Try to replace this with e.ensure
        Statemachine::StateTiming* timing;
        timing = e.get_mut<Statemachine::StateTiming, TemplateState>();
        if (!timing) {
            e.set<Statemachine::StateTiming, TemplateState>({0, 0});
            timing = e.get_mut<Statemachine::StateTiming, TemplateState>();
        }
        // Reset how long we've been in this state
        timing->timeInState_s = 0;
        e.add<TemplateState>();
    }
    void exit(Control& control) {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        fsmLogger->trace("Pawn {} exiting state {}", std::string(e.path()), Statemachine::TypeName<TemplateState>());
        e.remove<TemplateState>();
    }
};

// As the `utility` function is only defined for utilitarian states, need a separate
//  base class for those states
template <typename TemplateState>
struct BaseUtilityState : BasePawnState<TemplateState> {
    float utility(const typename PawnFSM::Control& control) const {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        const Statemachine::StateUtility* util = e.get<Statemachine::StateUtility, TemplateState>();
        if (util != nullptr) {
            return util->utility;
        } else {
            return 0.0f;
        }
    }
};


// Overall Pawn States

struct Alive : BaseUtilityState<Alive> {
};

struct Idle : BaseUtilityState<Idle> {
    void react(const Destination_Event& event, FullControl& control);
};

struct Working : BaseUtilityState<Working> {
};

struct Walking : BaseUtilityState<Walking> {
    void react(const Destination_Event& event, FullControl& control);
    void react(const Arrived_Event&, FullControl& control);
};

struct Combat : BaseUtilityState<Combat> {
    void update(FullControl& control) {};
};

struct Fleeing : BaseUtilityState<Fleeing> {
};

struct Dead : BaseUtilityState<Dead> {
};


// Occupation Pawn States
struct PawnOccupationUnemployed : BasePawnState<PawnOccupationUnemployed> {
};

struct PawnOccupationWoodcutter : BasePawnState<PawnOccupationWoodcutter> {
};

struct PawnWoodcutterStateWalkingTo : BasePawnState<PawnWoodcutterStateWalkingTo> {
    void react(const Arrived_Event&, FullControl& control);
};

struct PawnWoodcutterStateReturning : BasePawnState<PawnWoodcutterStateReturning> {
};

struct PawnWoodcutterStateChopping : BasePawnState<PawnWoodcutterStateChopping> {
};






struct PawnFSMContainer {
    std::shared_ptr<PawnFSM::Instance> machine;
};



}