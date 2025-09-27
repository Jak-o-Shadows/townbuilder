#include <tracy/Tracy.hpp>

#include "statemachine/module.hpp"

#include "msgLogging/module.hpp"


namespace Statemachine {

std::shared_ptr<spdlog::logger> componentsLogger;

components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    componentsLogger->trace("Module Created");

    ecs.component<CurveFile>("CurveFile")
        .member<std::string>("filename");
    ecs.component<Curve>();
    ecs.component<StateTiming>()
        .member<float>("timeInState_s")
        .member<float>("culmulativeTimeInState_s");
    ecs.component<StateUtility>()
        .member<float>("utility");
    componentsLogger->trace("Components Registered");


};


}