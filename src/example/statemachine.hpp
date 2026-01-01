#pragma once

#define HFSM2_ENABLE_ALL
#include <hfsm2/machine.hpp>

#include "msgLogging/module.hpp"
#include "statemachine/module.hpp"


namespace Example {

extern std::shared_ptr<spdlog::logger> statemachineLogger;

// Reactions
struct EventA {};
struct EventB {
    int value;
};

// states need to be forward declared to be used in FSM struct declaration.
//    This also allows them to be used as tags in flecs
struct State1;
struct State2;
struct State3;
struct State4;

// Define the FSM
// convenience typedef
using M = hfsm2::MachineT<hfsm2::Config::ContextT<Statemachine::Context>>;
using FSM = M::PeerRoot<
                // sub-machine ..
                M::Utilitarian<State1,
                    // .. with 2 sub-states
                    State2,
                    State3
                >,
                State4
            >;

struct FSMContainer {
    std::shared_ptr<FSM::Instance> machine;
};

template <typename TemplateState>
struct BaseState : FSM::State {
    // BaseState is a base class for all states, providing default reactions
    void react(const EventA&, FullControl& control){};  // Despite what you think, this function doesn't seem to actually be called? But if not efined, causes a compiler issue
    using FSM::State::react;
    // and default enter and exit
    void enter(Control& control) {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        statemachineLogger->trace("{} entering state {}", std::string(e.path()), Statemachine::TypeName<TemplateState>());
        Statemachine::StateTiming& timing = e.ensure<Statemachine::StateTiming, TemplateState>();
        timing.timeInState_s = 0;
        e.add<TemplateState>();
    }
    void exit(Control& control) {
        flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
        statemachineLogger->trace("{} exiting state {}", std::string(e.path()), Statemachine::TypeName<TemplateState>());
        e.remove<TemplateState>();
    }
};

struct State1 : BaseState<State1> {
    using BaseState<State1>::react;
};
struct State2 : BaseState<State2> {
    void react(const EventA&, EventControl& control);
    using BaseState<State2>::react;
};
struct State3 : BaseState<State3> {
    void react(const EventB&, EventControl& control);
    using BaseState<State3>::react;
};
struct State4 : BaseState<State4> {
    void react(const EventA&, EventControl& control);
    using BaseState<State4>::react;
};



}