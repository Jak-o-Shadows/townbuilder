#pragma once

#include <flecs.h>
#include <future>
#include <functional>
#include <string>
#include <chrono>

namespace Async {

/**
 * @brief A component to hold the future for a pending async task.
 * @tparam ResultType The type of the result the future will hold.
 */
template<typename ResultType>
struct Future {
    std::future<ResultType> future;
};

/**
 * @brief A helper function to create a pair of systems that manage an asynchronous task.
 *
 * This function abstracts the pattern of "fire-and-forget" async tasks in flecs.
 * It creates two systems:
 * 1. An observer that triggers when a `RequestComponent` is added to an entity. It
 *    runs the `work_function` in a separate thread via std::async.
 * 2. A polling system that periodically checks for completed futures. When a result
 *    is ready, it calls the `result_function` with the entity and the result.
 *
 * @tparam RequestComponent The component that signals a request for the async work.
 * @tparam ResultType The type of data the async work will return.
 * @tparam WorkFn The type of the function that performs the long-running computation.
 *         It must be invocable with `(flecs::entity)` and return `ResultType`.
 * @tparam ResultFn The type of the function that processes the result.
 *         It must be invocable with `(flecs::entity, ResultType)`.
 *
 * @param world The flecs world.
 * @param name A base name for the created systems.
 * @param work_function The function to execute asynchronously.
 * @param result_function The function to call with the result on the main thread.
 * @param tick_source (Optional) The tick source entity for the polling system. If 0,
 *                    the system uses the default tick source (runs every frame).
 */
template<typename RequestComponent, typename ResultType, typename WorkFn, typename ResultFn>
void create_async_system(
    flecs::world& world,
    const char* name,
    WorkFn work_function,
    ResultFn result_function,
    flecs::entity tick_source = {}
) {
    // Use the request component's name to create a unique future component name
    const char* request_name = world.component<RequestComponent>().name();
    std::string future_name = std::string("Async::Future<") + request_name + ">";
    flecs::component<Async::Future<ResultType>> future_component = world.component<Async::Future<ResultType>>(future_name.c_str());

    // System 1: Observer to kick off the task
    std::string start_name = std::string(name) + "_Start";
    world.observer<const RequestComponent>(start_name.c_str())
        .event(flecs::OnSet)
        .not_if(future_component) // Don't start if a task is already running
        .each(= {
            // Launch the async task, capturing necessary data.
            // The work_function is expected to handle its own data needs.
            std::future<ResultType> fut = std::async(std::launch::async, work_function, e);

            // Add the future component to the entity to track it.
            e.set<Async::Future<ResultType>>(future_component, {std::move(fut)});
            // The request has been handled, so we can remove it.
            e.remove<RequestComponent>();
        });

    // System 2: Polling system to check for results
    std::string check_name = std::string(name) + "_Check";
    flecs::system<Async::Future<ResultType>> sys = world.system<Async::Future<ResultType>>(check_name.c_str())
        .term_at(0).id(future_component); // Ensure we query for the correct future type

    if (tick_source.is_valid()) {
        sys.tick_source(tick_source);
    }
    
    sys.each(= {
            if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                ResultType result = fut_comp.future.get();
                result_function(e, std::move(result));
                e.remove(future_component);
            }
        });
}

} // namespace Async