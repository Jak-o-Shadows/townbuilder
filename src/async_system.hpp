#pragma once

#include <flecs.h>

#include <future>
#include <functional>
#include <tuple>
#include <string>
#include <chrono>
#include <format>

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
 * @brief Creates a system to manage asynchronous tasks with automatic data marshalling.
 *
 * This function sets up two systems for handling "fire-and-forget" asynchronous operations:
 * 1. A "start" system that observes when a `RequestComponent` is added to an entity.
 *    It safely copies data from `InputComponents...` and launches the `work_function`
 *    on a background thread.
 * 2. A "check" system that polls for completed tasks. When a task finishes, it
 *    takes the `std::tuple<ResultComponents...>` returned by the work function and
 *    sets each element as a component on the entity.
 *
 * This design ensures that the worker function is pure and thread-safe, as it
 * operates on copies of the data and does not need access to the world.
 *
 * @tparam RequestComponent Component that triggers the async task. Can be a tag or a component with data.
 * @tparam ResultComponents... A pack of component types that the work function will return as a std::tuple.
 * @tparam InputComponents... A pack of component types to be copied and passed as arguments to the work function.
 *
 * @param world The flecs::world instance.
 * @param work_function The function to execute on a background thread.
 *        Signature: `std::tuple<ResultComponents...>(const InputComponents&...)`
 * @param tick_source The tick source for the systems.
 * @param system_name_prefix A name for the system, used for debugging and identification.
 */
template<typename... ResultComponents, typename... InputComponents>
void create_async_system(
    flecs::world& world,
    std::function<std::tuple<ResultComponents...>(const InputComponents&...)> work_function,
    flecs::entity tick_source = {},
    std::string system_name_prefix
) {
    using ResultTuple = std::tuple<ResultComponents...>;
    using InputTuple = std::tuple<InputComponents...>;

    // Use the request component's name to create a unique future component name
    std::string future_name = std::format("{}_Future", system_name_prefix);
    auto future_component = world.component<Async::Future<ResultTuple>>(future_name.c_str());

    // System 1: Kicks off the async task.
    // It queries for entities with all InputComponents,
    // but without an existing future, to prevent re-launching a task.
    std::string start_name = std::format("{}_Start", system_name_prefix);
    world.system<const InputComponents...>(start_name.c_str())
        .without(future_component) // Don't start if a task is already running
        .tick_source(tick_source)
-       .each([=](flecs::entity e, const InputComponents&... inputs) {
            // Create a tuple of copies of the input components. This is the key
            // to making the worker function thread-safe.
            auto inputs_tuple = std::make_tuple(others...);

            // Launch the async task. `std::apply` unpacks the tuple of inputs
            // into arguments for the work_function.
            std::future<ResultTuple> fut = std::async(std::launch::async, [=] {
                return std::apply(work_function, inputs_tuple);
            });

            // Add the future component to the entity to track it.
            e.set<Async::Future<ResultTuple>>({std::move(fut)});
        });

    // System 2: Polls for completed futures and applies the results.
    std::string check_name = std::format("{}_Check", system_name_prefix);
    world.system<Async::Future<ResultTuple>>(check_name.c_str())
        .tick_source(tick_source)
        .each([=](flecs::entity e, Async::Future<ResultTuple>& fut_comp) {
            if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                ResultTuple result_tuple = fut_comp.future.get();

                // Helper to set components from the result tuple.
                // We use a fold expression over a comma operator to iterate the tuple.
                // This is a C++17 trick.
                ( (e.set(std::get<ResultComponents>(result_tuple))), ... );

                // The task is complete, so remove the request and future components.
                // The slightly not great performance of removing components is ok
                // here since by definition this should be a low frequency operation
                e.remove(future_component);
            }
        });
}

} // namespace Async
