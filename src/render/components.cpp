#include "render/module.hpp"

#include <tracy/Tracy.hpp>
#include <spdlog/spdlog.h>

#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"
#include "flecs_components_physics.h"
#include "flecs_components_gui.h"
#include "flecs_systems_transform.h"

#include "imgui.h"
#include "../bindings/imgui_impl_glfw.h"
#include "../bindings/imgui_impl_opengl3.h"



#include "pawn/module.hpp"
#include "buildings/module.hpp"
#include "map/module.hpp"
#include "ticks/module.hpp"
#include "pathfinding/module.hpp"
#include "render/renderNavmesh.hpp"
#include "coordinates/module.hpp"

namespace Render{

std::shared_ptr<spdlog::logger> componentsLogger;








components::components(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<components>();
    componentsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::err});
    componentsLogger->trace("Module Created");
    std::cout << "Render Module Created" << std::endl;


    ecs.component<ImColor>()
        .member<float>("r")
        .member<float>("g")
        .member<float>("b")
        .member<float>("a");
    ecs.component<Box>()
        .member<float>("width")
        .member<float>("height")
        .member<float>("depth");
    ecs.component<flecs::components::transform::Position3>()  // TODO: We shouldn't have to be registering this. Am I jsut not importing it?
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");
    ecs.component<Window>()
        .add(flecs::Singleton);
    componentsLogger->trace("Components Registered");


};


}