#include <tracy/Tracy.hpp>

#include "example/module.hpp"

#include "msgLogging/module.hpp"
#include "statemachine/module.hpp"

#define HFSM2_ENABLE_ALL
#include <hfsm2/machine.hpp>

#include <iostream>

namespace Example {

std::shared_ptr<spdlog::logger> statemachineLogger;


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

// Specific state reactions
struct State1 : BaseState<State1> {
    using BaseState<State1>::react;
};
struct State2 : BaseState<State2> {
    void react(const EventA&, EventControl& control) {
        control.changeTo<State3>();
    };
    using BaseState<State2>::react;
};
struct State3 : BaseState<State3> {
    void react(const EventB&, EventControl& control) {
        control.changeTo<State4>();
    };
    using BaseState<State3>::react;
};
struct State4 : BaseState<State4> {
    void react(const EventA&, EventControl& control) {
        control.changeTo<State1>();
    };
    using BaseState<State4>::react;
};





// Finallly, the system

statemachine::statemachine(flecs::world& ecs) {
    flecs::entity m = ecs.module<statemachine>();
    statemachineLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    statemachineLogger->trace("Example statemachine module created");

    

    ecs.system<FSMContainer>("System_Statemachine_EventA")
        .term_at(0).inout()
        .interval(5)
        .each([](flecs::entity e, FSMContainer& fsmc){
            ZoneScopedN("System_Statemachine_EventA");
            fsmc.machine->react(EventA{});
        });
    ecs.system<FSMContainer>("System_Statemachine_EventB")
        .term_at(0).inout()
        .interval(2.5)
        .each([](flecs::entity e, FSMContainer& fsmc){
            ZoneScopedN("System_Statemachine_EventB");
            fsmc.machine->react(EventB{42});
        });
    statemachineLogger->trace("Systems created");
    

    flecs::entity e = ecs.entity("StateMachinerTestEntity");
    Statemachine::Context blah{e.id(), ecs};  // No idea why this has to be a separate variable, but it does, so bugger it
    e.set<FSMContainer>({std::shared_ptr<FSM::Instance>(new FSM::Instance(blah))});
    statemachineLogger->trace("Entities created");

    
}

}