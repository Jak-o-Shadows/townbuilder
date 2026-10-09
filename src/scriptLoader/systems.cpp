#include "scriptLoader/module.hpp"

#include "msgLogging/module.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <iostream>

namespace ScriptLoader {

static std::shared_ptr<spdlog::logger> systemsLogger;

systems::systems(flecs::world& ecs) {
    flecs::entity m = ecs.module<systems>();

    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Systems module created");

    ecs.system<ScriptsToLoad>("LoadFlecsScripts")
        .kind(flecs::OnStart)  // Only run the once at startup. Cannot be an observer as it runs before the database is setup
        .each([&ecs](ScriptsToLoad& scripts) {
            systemsLogger->trace("Entering LoadFlecsScripts, loaded={}", scripts.loaded);
            if (scripts.loaded) {
                return;
            }

            for (const auto& filepath : scripts.filepaths) {
                systemsLogger->trace("LoadFlecsScripts: running script file {}", filepath);
                std::cout << std::format("LoadFlecsScripts: running script file {}", filepath) << std::endl;
                ecs_script_run_file(ecs, filepath.c_str());
                systemsLogger->debug("LoadFlecsScripts: finished script file {}", filepath);
                std::cout << std::format("LoadFlecsScripts: finished script file {}", filepath) << std::endl;
            }
            scripts.loaded = true;
            systemsLogger->trace("Exiting LoadFlecsScripts");
        });
}

}