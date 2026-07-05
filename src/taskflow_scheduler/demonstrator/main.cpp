#include "taskflow_scheduler/module.hpp"
#include "taskflow_scheduler/scheduler.hpp"
#include "msgLogging/module.hpp"

#include <flecs.h>
#include <flecs/addons/meta.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <iostream>

struct DataComponentA {
    float value;
};

struct DataComponentB {
    int value;
};

std::shared_ptr<spdlog::logger> logger;

int main(int, char *[]) {
    std::cout << "Starting main" << std::endl;

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::stats>(); // Enable statistics in explorer
    std::cout << "World created" << std::endl;

    // Logger imported first as the other modules use it on their import
    ecs.import<Logging::components>();
    ecs.import<Logging::systems>();
    std::cout << "Logger imported" << std::endl;

    // Setup a non-module logger for main.cpp
    auto sinks = ecs.get<Logging::LoggerSink>().sinks;
    logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::debug);  // Set to trace in the knowledge that the observer will fire soon and change it
    spdlog::register_logger(logger);

    logger->info("Main logger created");

    ecs.import<TaskflowScheduler::components>();
    ecs.import<TaskflowScheduler::systems>();

    logger->info("Modules imported");

    // Demonstrator Components
    ecs.component<DataComponentA>("DataComponentA")
        .member("value", &DataComponentA::value);
    ecs.component<DataComponentB>("DataComponentB")
        .member("value", &DataComponentB::value);

    logger->info("Components created");

    // Demonstrator Systems
    ecs.system<DataComponentA>("ProducerSystemA")
        .term_at(0).inout()
        .each([](DataComponentA& data) {
            ZoneScopedN("ProducerSystemA");
            data.value += 1.0f;
            logger->trace("Produced: {}", data.value);
        });

    ecs.system<const DataComponentA>("ConsumerSystemA")
        .term_at(0).in()
        .each([](const DataComponentA& data) {
            ZoneScopedN("ConsumerSystemA");
            logger->trace("Consumed: {}", data.value);
        });

    ecs.system<DataComponentB>("ProducerSystemB")
        .term_at(0).inout()
        .each([](DataComponentB& data) {
            ZoneScopedN("ProducerSystemB");
            data.value += 1;
            logger->trace("Produced: {}", data.value);
        });

    ecs.system<const DataComponentB>("ConsumerSystemB")
        .term_at(0).in()
        .each([](const DataComponentB& data) {
            ZoneScopedN("ConsumerSystemB");
            logger->trace("Consumed: {}", data.value);
        });

    ecs.system<const DataComponentA, const DataComponentB>("CombinedConsumerSystem")
        .term_at(0).in()
        .term_at(1).in()
        .each([](const DataComponentA& dataA, const DataComponentB& dataB) {
            ZoneScopedN("CombinedConsumerSystem");
            logger->trace("Consumed A: {}, B: {}", dataA.value, dataB.value);
        });

    logger->info("Systems created");


    // Create an entity with the component
    flecs::entity e = ecs.entity("DataEntity")
        .set<DataComponentA>({0.0f})
        .set<DataComponentB>({0})
        ;

    logger->info("Entities created");


    logger->info("Starting main loop");
    ecs.set_target_fps(1);
    while (true) {
        FrameMarkNamed("Frame");
        ecs.progress();
    }

}