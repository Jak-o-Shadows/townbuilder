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
#include "coordinates/module.hpp"

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



struct PawnOccupying {};

struct Likes { };


extern std::shared_ptr<spdlog::logger> fsmLogger;







// data shared between FSM states and outside code

// convenience typedef
using M = hfsm2::MachineT<hfsm2::Config::ContextT<Statemachine::Context>>;


// Events
struct Dummy_Event {};
struct Destination_Event {
    Coordinates::Grid target;
    Coordinates::Cell local;
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
    // BasePawnState is a base class for all Pawn states, providing the default entry, exit
    void react(const Dummy_Event& event, FullControl& control) {};  // Doesn't seem to be called, but fixes compiler stuff?
    void react(const Attacked& event, FullControl& control) {std::cout << "base attacked" << std::endl;};
    using PawnFSM::State::react;

    void enter(Control& control) {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        fsmLogger->trace("Pawn {} entering state {}", std::string(e.path()), Statemachine::TypeName<TemplateState>());
          // TODO: Try to replace this with e.ensure - UPDATE 2026-01-01 - ensure doesn't work with relationships?
        Statemachine::StateTiming* timing;
        timing = e.try_get_mut<Statemachine::StateTiming, TemplateState>();
        if (!timing) {
            fsmLogger->trace("Creating timing data for Pawn {} state {}", std::string(e.path()), Statemachine::TypeName<TemplateState>());
            // By zero-initialising it, we can just skip having to modify it after.
            //  this is important because this reaction may be called from a system which defers
            //  ECS changes - and hence we may not be able to set the component,
            //  and then modify it with a get_mut right after
            e.set<Statemachine::StateTiming, TemplateState>({0, 0});
        } else {
            // Reset how long we've been in this state
            timing->timeInState_s = 0;
        }
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
//  Note that utilitarian states MUST use EventControl for their reactions; FullControl DOES NOT work (compiles, but never called)
template <typename TemplateState>
struct BaseUtilityState : BasePawnState<TemplateState> {
    using BasePawnState<TemplateState>::react;
    float utility(const typename PawnFSM::Control& control) const {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        const Statemachine::StateUtility* util = e.try_get<Statemachine::StateUtility, TemplateState>();  // try_get so I can be lazy and not have it defined for all
        if (util != nullptr) {
            return util->utility;
        } else {
            return 0.0f;
        }
    }
};

// Overall Pawn States
// Explicitly bring the base class's react methods into this scope
// to resolve ambiguity for the compiler.
struct Alive : BaseUtilityState<Alive> {
    void react(const Arrived_Event& event, EventControl& control);
    void react(const Attacked& event, EventControl& control);
    using BaseUtilityState<Alive>::react;
};

struct Idle : BaseUtilityState<Idle> {
    using BaseUtilityState<Idle>::react;
};

struct Working : BaseUtilityState<Working> {
    using BaseUtilityState<Working>::react;
};

struct Walking : BaseUtilityState<Walking> {
    void react(const Destination_Event& dest, EventControl& control);
    void react(const Arrived_Event& event, EventControl& control);
    using BaseUtilityState<Walking>::react;
};

struct Combat : BaseUtilityState<Combat> {
    using BaseUtilityState<Combat>::react;
};

struct Fleeing : BaseUtilityState<Fleeing> {
    using BaseUtilityState<Fleeing>::react;
};

struct Dead : BasePawnState<Dead> {
    using BasePawnState<Dead>::react;
};


// Occupation Pawn States
struct PawnOccupationUnemployed : BasePawnState<PawnOccupationUnemployed> {
    using BasePawnState<PawnOccupationUnemployed>::react;
};

struct PawnOccupationWoodcutter : BasePawnState<PawnOccupationWoodcutter> {
    using BasePawnState<PawnOccupationWoodcutter>::react;
};

struct PawnWoodcutterStateWalkingTo : BasePawnState<PawnWoodcutterStateWalkingTo> {
    using BasePawnState<PawnWoodcutterStateWalkingTo>::react;
};

struct PawnWoodcutterStateReturning : BasePawnState<PawnWoodcutterStateReturning> {
    using BasePawnState<PawnWoodcutterStateReturning>::react;
};

struct PawnWoodcutterStateChopping : BasePawnState<PawnWoodcutterStateChopping> {
    using BasePawnState<PawnWoodcutterStateChopping>::react;
};






struct PawnFSMContainer {
    std::shared_ptr<PawnFSM::Instance> machine;
};



}