#include <tracy/Tracy.hpp>

#include "example/statemachine.hpp"
#include "example/module.hpp"

#include <iostream>


namespace Example {

// Extern defn
std::shared_ptr<spdlog::logger> statemachineLogger;



// Specific state reactions

void State2::react(const EventA& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    statemachineLogger->info("State2::react(EventA) called for entity {}", std::string(e.path()));
    control.changeTo<State3>();
}

void State3::react(const EventB& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    statemachineLogger->info("State3::react(EventB) called for entity {}", std::string(e.path()));
    control.changeTo<State4>();
}

void State4::react(const EventA& event, EventControl& control) {
    flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
    statemachineLogger->info("State4::react(EventA) called for entity {}", std::string(e.path()));
    control.changeTo<State1>();
}



// Finallly, the system

statemachine::statemachine(flecs::world& ecs) {
    flecs::entity m = ecs.module<statemachine>();
    statemachineLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    statemachineLogger->trace("Example statemachine module created");

    

    ecs.system<FSMContainer>("System_Statemachine_EventA")
        .term_at(0).inout()
        .interval(3.5)
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