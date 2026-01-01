#include "example/module.hpp"

#include "msgLogging/module.hpp"
#include "async_system.hpp"
#include "ticks/module.hpp"

#include <spdlog/spdlog.h>

namespace Example {

std::shared_ptr<spdlog::logger> asyncLogger;


struct AsyncSingletonTestInputData{
    float value;
};

struct AsyncSingletonTestOutputData{
    std::string msg;
};

std::tuple<AsyncSingletonTestOutputData> async_singleton_test_work(const AsyncSingletonTestInputData& req) {
    ZoneScopedN("async_singleton_test_work");
    // Simulate long work
    //std::cout << "[Async] Starting long work with value " << req.value << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    //std::cout << "[Async] Finished long work." << std::endl;
    return { { "Processed value: " + std::to_string(req.value) } };
}


struct AsyncTestInputData {
    float value;
};

struct AsyncTestOutputData {
    std::string msg;
};
struct AsyncTestOutputData2 {
    std::string msg;
};
/**
 * @brief A standalone function to perform long-running work for the async system example.
 * 
 * @param req The input data component, passed by const reference.
 * @return A std::tuple containing the result component(s).
 */
std::tuple<AsyncTestOutputData> async_test_work(const AsyncTestInputData& req) {
    ZoneScopedN("async_test_work");
    // Simulate long work
    //std::cout << "[Async] Starting long work with value " << req.value << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    //std::cout << "[Async] Finished long work." << std::endl;
    return { { "Processed value: " + std::to_string(req.value) } };
}
std::tuple<AsyncTestOutputData2> async_test_work2(const AsyncTestInputData& req) {
    ZoneScopedN("async_test_work2");
    // Simulate long work
    //std::cout << "[Async2] Starting long work with value " << req.value << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    //std::cout << "[Async2] Finished long work." << std::endl;
    return { { "Processed value: " + std::to_string(req.value) } };
}


async::async(flecs::world& ecs) {
    flecs::entity m = ecs.module<async>();
    asyncLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    asyncLogger->trace("Example async module created");

    ecs.component<AsyncSingletonTestInputData>("AsyncSingletonTestInputData")
        .member<float>("value")
        .add(flecs::Singleton);
    ecs.component<AsyncSingletonTestOutputData>("AsyncSingletonTestOutputData")
        .member<std::string>("msg")
        .add(flecs::Singleton);
    ecs.component<AsyncTestInputData>("AsyncTestInputData")
        .member<float>("value");
    ecs.component<AsyncTestOutputData>("AsyncTestOutputData")
        .member<std::string>("msg");
    ecs.component<AsyncTestOutputData2>("AsyncTestOutputData2")
        .member<std::string>("msg");
    asyncLogger->info("Components Registered");

    ecs.observer<AsyncTestInputData, const AsyncTestOutputData>("Observer_AsyncTestInputDataIncrement")
        .term_at(0).inout()
        .term_at(1).in()
        .event(flecs::OnSet)
        .each([](flecs::entity e, AsyncTestInputData& data, const AsyncTestOutputData&) {
            data.value += 1;
        })
        .set_doc_brief("Increment the test async input to show that change is working");
    asyncLogger->info("Regular Systems created");


// Example of the async system
    std::function<std::tuple<AsyncTestOutputData>(const AsyncTestInputData&)> work_fn = async_test_work;
    Async::create_async_system(ecs, "AsyncTestSystem")
        .query<const AsyncTestInputData>()
        .work(work_fn)
        .tick_source(Ticks::tick_ui)
        .build();
    std::function<std::tuple<AsyncTestOutputData2>(const AsyncTestInputData&)> work_fn2 = async_test_work2;
    Async::create_async_system(ecs, "AsyncTestSystem2")
        .query<const AsyncTestInputData>()
        .work(work_fn2)
        .tick_source(Ticks::tick_ui)
        .build();




    // Test the singleton async
    std::function<std::tuple<AsyncSingletonTestOutputData>(const AsyncSingletonTestInputData&)> work_fn3 = async_singleton_test_work;
    Async::create_async_system_for_singleton(ecs, "AsyncSingletonTestSystem")
        .query<const AsyncSingletonTestInputData>()
        .work(work_fn3)
        .tick_source(Ticks::tick_ui)
        .build();
    ecs.set<AsyncSingletonTestInputData>({13.0f});

    asyncLogger->trace("Async systems created");






    auto qb = ecs.query_builder<const AsyncTestInputData>()
        .term_at(0).up(); // Query for entities that have a child with AsyncTestInputData
//    Async::create_async_system_with_query<AsyncTestOutputData>(
//        ecs,
//        qb,
//        work_fn,
//        Ticks::tick_ui,
//        "AsyncTestSystemWithQuery"
//    );

    /*
    flecs::query<const AsyncTestInputData> q = qb.build();
    ecs.system("Test System")
        .tick_source(Ticks::tick_ui)
        .run([q](flecs::iter it){
            std::cout << "Test System running, found " << q.count() << " entities with AsyncTestInputData" << std::endl;
            q.each([](flecs::entity e, const AsyncTestInputData& data){
                std::cout << " - Entity " << std::string(e.path()) << " has AsyncTestInputData.value = " << data.value << std::endl;
            });
        });
    */

    flecs::entity async_tester = ecs.entity("AsyncTester")
        .set<AsyncTestInputData>({42.0f});
    ecs.entity("AsyncTestChild")
        .child_of(async_tester);
    asyncLogger->info("Entities created");



}

}