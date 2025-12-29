#include "pythonEcsbinding/module.hpp"

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"

#include <tracy/Tracy.hpp>

#include <iostream>
 


 
namespace Python {
 
std::shared_ptr<spdlog::logger> systemsLogger;
std::vector<pybind11::subinterpreter> interpreters;


systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    // Create the main Python Interpreter. Each file we import then uses
    // a separate sub-interpreter. This gives safety between python files, and
    // also allows concurrency between the files
    pybind11::initialize_interpreter();
    

    systemsLogger->trace("Python main interpreter initialized");

    // Register systems
    ecs.observer<PythonFile>("InitialisePythonFile")
        .event(flecs::OnAdd)
        .each([](flecs::entity e, PythonFile& pf) {
            ZoneScopedN("InitialisePythonFile");
            interpreters.emplace_back(pybind11::subinterpreter().create());
            pf.interpreter_idx = interpreters.size() - 1;
            pybind11::subinterpreter_scoped_activate (interpreters.at(pf.interpreter_idx));
            systemsLogger->trace("Interpreter created for PythonFile on entity {}", std::string(e.path()));
        });

    ecs.system<PythonFile>("TickPythonFile")
        .tick_source(Ticks::tick_python)
        .each([](flecs::entity e, PythonFile& pf) {
            ZoneScopedN("TickPythonFile");
            systemsLogger->trace("Ticking PythonFile on entity {}", std::string(e.path()));
            pybind11::subinterpreter_scoped_activate (interpreters.at(pf.interpreter_idx));
            pybind11::eval_file(pf.filepath);
        });

    systemsLogger->trace("Systems Registered");

};
 
}
