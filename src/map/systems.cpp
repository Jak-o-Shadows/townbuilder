#include "map/module.hpp"

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"
#include "buildings/module.hpp"

#include <tracy/Tracy.hpp>

#include <math.h>
#include <iostream>

namespace Map {

std::shared_ptr<spdlog::logger> systemsLogger;


bool is_empty_resources(const Buildings::Resources& resources){
    return (resources.fish <= 0)
        && (resources.stone <= 0)
        && (resources.wood <= 0);
}




systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<Ticks::module>();
    ecs.import<Buildings::components>();




    ecs.system<const Buildings::Resources>("System_KillTrees")
        .term_at(0).in()
        .with(flecs::IsA, ecs.id<Map::Tree_Prefab>())
        .each([](flecs::entity tree, const Buildings::Resources& resources){
            ZoneScopedN("System_KillTrees");
            if (is_empty_resources(resources)) {
                tree.destruct();
            }
        });

}

}