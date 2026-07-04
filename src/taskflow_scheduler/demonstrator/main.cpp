#include "taskflow_scheduler/module.hpp"

#include <flecs.h>
#include <flecs/addons/meta.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <iostream>

int main(int, char *[]) {
    std::cout << "Starting main" << std::endl;

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::stats>(); // Enable statistics in explorer
    std::cout << "World created" << std::endl;

    ecs.import<TaskflowScheduler::components>();
    ecs.import<TaskflowScheduler::systems>();

    std::cout << "Just before run" << std::endl;
    while (true) {
        ecs.progress();
        FrameMarkNamed("Frame");
    }


}