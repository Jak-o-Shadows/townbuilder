#pragma once

#include <flecs.h>
#include <tracy/Tracy.hpp>

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

// Helper to deduce argument types from a std::function
template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    using result_type = R;
    using argument_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
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
 * @tparam InputComponents... A pack of component types that trigger the async task when on an entity
 * @tparam ResultComponents... A pack of component types that the work function will return as a std::tuple.
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
    flecs::entity tick_source,
    const std::string system_name_prefix
) {
    ZoneScopedN("create_async_system");
    using ResultTuple = std::tuple<ResultComponents...>;
    static_assert(sizeof...(InputComponents) > 0, "Async system must have at least one input component to act as a trigger.");
    //std::cout << std::format("Registering async system '{}'", system_name_prefix) << std::endl;
    // Use the request component's name to create a unique future component name
    const std::string future_name = std::format("{}_Future", system_name_prefix);
    flecs::entity future_component = world.component<Async::Future<ResultTuple>>(future_name.c_str());
    //std::cout << std::format("Future component '{}' registered", future_name) << std::endl;

    // System 1: Kicks off the async task.
    // It queries for entities with all InputComponents,
    // but without an existing future, to prevent re-launching a task.
    const std::string start_name = std::format("{}_Start", system_name_prefix);
    world.system<const InputComponents...>(start_name.c_str())
        .without(future_component) // Don't start if a task is already running
        .tick_source(tick_source)
        .each([=](flecs::entity e, const InputComponents&... inputs) {
            ZoneScoped; ZoneName(start_name.c_str(), start_name.length());
            //std::cout << std::format("Starting async task for entity {}", std::string(e.path())) << std::endl;
            // Create a tuple of copies of the input components. This is the key
            // to making the worker function thread-safe.
            auto inputs_tuple = std::make_tuple(InputComponents(inputs)...);
            //std::cout << "Input components copied for async task." << std::endl;

            // Launch the async task. `std::apply` unpacks the tuple of inputs
            // into arguments for the work_function. The lambda wrapper is necessary
            // to defer the execution of std::apply to the new thread.
            std::future<ResultTuple> fut = std::async(std::launch::async, [=] {
                return std::apply(work_function, inputs_tuple);
            });
            //std::cout << "Async task launched." << std::endl;

            // Add the future component to the entity to track it.
            e.set<Async::Future<ResultTuple>>({std::move(fut)});
        });
    //std::cout << std::format("Start system {} registered", start_name) << std::endl;

    
    // System 2: Polls for completed futures and applies the results.
    const std::string check_name = std::format("{}_Check", system_name_prefix);
    world.system<Async::Future<ResultTuple>>(check_name.c_str())
        .tick_source(tick_source)
        .each([=](flecs::entity e, Async::Future<ResultTuple>& fut_comp) {
            ZoneScoped; ZoneName(check_name.c_str(), check_name.length());
            //std::cout << std::format("Checking async task for entity {}", std::string(e.path())) << std::endl;
            if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                //std::cout << "Async task completed, retrieving results." << std::endl;
                ResultTuple result_tuple = fut_comp.future.get();
                //std::cout << "Results retrieved from async task." << std::endl;

                // Set all the result components on the entity.
                // We use std::apply to unpack the result tuple and set each component.
                std::apply([&](const auto&... components) {
                    (e.set(components), ...);
                }, result_tuple);
                //std::cout << "Result components set on entity." << std::endl;

                // The task is complete, so remove the trigger and future components.
                // The slightly not great performance of removing components is ok
                // here since by definition this should be a low frequency operation
                e.remove(future_component);
            }
        });
    //std::cout << std::format("Check system {} registered", check_name) << std::endl;
    
}

/**
 * @brief An alternative version of `create_async_system` that accepts a user-defined query.
 *
 * This version allows for more complex entity selection logic by letting the user
 * provide a pre-configured `flecs::query_builder`. The system will be built
 * from this query builder. The `InputComponents...` must match the signature
 * of the query builder and the `work_function`.
 *
 * @tparam ResultComponents... A pack of component types that the work function will return as a std::tuple.
 * @tparam InputComponents... A pack of component types to be queried and passed to the work function.
 *
 * @param world The flecs::world instance.
 * @param query_builder A `flecs::query_builder` that defines which entities to select.
 * @param work_function The function to execute on a background thread.
 * @param tick_source The tick source for the systems.
 * @param system_name_prefix A name for the system, used for debugging and identification.
 */
template<typename... ResultComponents, typename... InputComponents, typename WorkFn>
void create_async_system_with_query(
    flecs::world& world,
    flecs::query_builder<InputComponents...>& query_builder,
    WorkFn work_function,
    flecs::entity tick_source,
    std::string system_name_prefix
) {
    using ResultTuple = std::tuple<ResultComponents...>;
    static_assert(sizeof...(InputComponents) > 0, "Async system must have at least one input component to act as a trigger.");

    std::string future_name = std::format("{}_Future", system_name_prefix);
    flecs::entity future_component = world.component<Async::Future<ResultTuple>>(future_name.c_str());

    // Build a query from the user-provided query_builder
    flecs::query<InputComponents...> start_query = query_builder.build();

    // Build the "start" system using the created query
    std::string start_name = std::format("{}_Start", system_name_prefix);
    flecs::system start_system = world.system(start_name.c_str())
        .without(future_component)
        .tick_source(tick_source)
        .run([=](flecs::iter&){
            std::cout << std::format("Running start system '{}'", start_name) << std::endl;
            start_query.each([=](flecs::entity e, const InputComponents&... inputs) {
                std::cout << std::format("Starting async task for entity {}", std::string(e.path())) << std::endl;
                // Create a tuple of copies of the input components. This is the key
                // to making the worker function thread-safe.
                auto inputs_tuple = std::make_tuple(InputComponents(inputs)...);
                std::cout << "Input components copied for async task." << std::endl;

                // Launch the async task. `std::apply` unpacks the tuple of inputs
                // into arguments for the work_function. The lambda wrapper is necessary
                // to defer the execution of std::apply to the new thread.
                std::future<ResultTuple> fut = std::async(std::launch::async, [=] {
                    return std::apply(work_function, inputs_tuple);
                });
                std::cout << "Async task launched." << std::endl;

                // Add the future component to the entity to track it.
                e.set<Async::Future<ResultTuple>>({std::move(fut)});
            });
        });

    // System 2: Polls for completed futures and applies the results.
    std::string check_name = std::format("{}_Check", system_name_prefix);
    world.system<Async::Future<ResultTuple>>(check_name.c_str())
        .tick_source(tick_source)
        .each([=](flecs::entity e, Async::Future<ResultTuple>& fut_comp) {
            //std::cout << std::format("Checking async task for entity {}", std::string(e.path())) << std::endl;
            if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                //std::cout << "Async task completed, retrieving results." << std::endl;
                ResultTuple result_tuple = fut_comp.future.get();
                //std::cout << "Results retrieved from async task." << std::endl;

                // Set all the result components on the entity.
                // We use std::apply to unpack the result tuple and set each component.
                std::apply([&](const auto&... components) {
                    (e.set(components), ...);
                }, result_tuple);
                //std::cout << "Result components set on entity." << std::endl;

                // The task is complete, so remove the trigger and future components.
                // The slightly not great performance of removing components is ok
                // here since by definition this should be a low frequency operation
                e.remove(future_component);
            }
        });
    //std::cout << std::format("Check system {} registered", check_name) << std::endl;


}


} // namespace Async
