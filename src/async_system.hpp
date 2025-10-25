#pragma once

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include <future>
#include <functional>
#include <tuple>
#include <string>
#include <utility>
#include <type_traits> // For std::is_empty_v

namespace Async {

// Helper to pass around type packs
template<typename... T>
struct type_list {};

// Helper to get traits from a std::function
template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    using result_type = R;
};

template<typename ResultType>
struct Future {
    std::future<ResultType> future;
};


// Forward declaration of the main builder
template <
    typename TQueryArgs,  // type_list of components to query for
    typename TGathered,   // A std::tuple<> of the data prepared by the gather stage
    typename TResults,    // A std::tuple<> of the results from the work stage
    typename TGatherFn,   // The gather function type
    typename TWorkFn,     // The work function type
    typename TApplyFn     // The apply function type
>
class AsyncSystemBuilder;

// Default Gather: Copy input components
struct DefaultGatherFn {
    template<typename... Args>
    std::tuple<Args...> operator()(flecs::entity, const Args&... args) const {
        return std::make_tuple(Args(args)...);
    }
};

// Default Apply: Set result components on the entity.
// Handles both tags (using add) and components with data (using set).
struct DefaultApplyFn {
private:
    template<typename T>
    void apply_result(flecs::entity e, const T& result) const {
        if constexpr (std::is_empty_v<T>) {
            e.add<T>();
        } else {
            e.set<T>(result);
        }
    }
public:
    template<typename... Results>
    void operator()(flecs::world&, flecs::entity e, const Results&... results) const {
        (apply_result(e, results), ...);
    }
};


// Entry point: returns a builder with default functions.
AsyncSystemBuilder<
    type_list<>, 
    type_list<>, 
    type_list<>, 
    DefaultGatherFn, 
    std::nullptr_t, 
    DefaultApplyFn
>
create_async_system(flecs::world& ecs, const char* name) {
    return AsyncSystemBuilder<
        type_list<>, 
        type_list<>, 
        type_list<>, 
        DefaultGatherFn, 
        std::nullptr_t, 
        DefaultApplyFn
    >(ecs, name, flecs::entity::null(), DefaultGatherFn{}, nullptr, DefaultApplyFn{});
}

/**
 * @brief A builder for creating efficient, thread-safe, single-system asynchronous operations.
 *
 * This builder constructs a flecs::system that follows a three-stage "gather-work-apply" pipeline:
 * 1. GATHER (Main Thread): A `gather` function runs inside a flecs system on the main thread.
 *    It safely extracts data from the ECS for each matching entity. By default, it just
 *    copies the queried components.
 * 2. WORK (Worker Thread): A `work` function runs on a background thread via std::async.
 *    It operates exclusively on the thread-safe data prepared by the `gather` stage.
 * 3. APPLY (Main Thread): An `apply` function runs on the main thread after the work is complete.
 *    It uses `ecs.defer()` to safely apply the results of the work back to the ECS. By default,
 *    it adds/sets any returned values as components to the entity.
 */
template <
    typename TQueryArgs,  // type_list of components to query for
    typename TGathered,   // A std::tuple<> of the data prepared by the gather stage
    typename TResults,    // A std::tuple<> of the results from the work stage
    typename TGatherFn,   // The gather function type
    typename TWorkFn,     // The work function type
    typename TApplyFn     // The apply function type
>
class AsyncSystemBuilder {
public:
    AsyncSystemBuilder(flecs::world& ecs, const char* name, flecs::entity tick_src,
                       TGatherFn gather_fn, TWorkFn work_fn, TApplyFn apply_fn)
        : world(ecs), system_name_prefix(name), tick_source_(tick_src),
          gather_fn_(gather_fn), work_fn_(work_fn), apply_fn_(apply_fn) {}

    // Define the components to query for.
    template<typename... Comps>
    AsyncSystemBuilder<type_list<Comps...>, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>
    query() {
        return AsyncSystemBuilder<type_list<Comps...>, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_source_, gather_fn_, work_fn_, apply_fn_
        );
    }

    // Set the system's tick source.
    AsyncSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>
    tick_source(flecs::entity tick_src) {
        return AsyncSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_src, gather_fn_, work_fn_, apply_fn_
        );
    }

    // (Optional) Provide a custom gather function.
    template<typename Fn>
    AsyncSystemBuilder<TQueryArgs, typename function_traits<decltype(std::function(std::declval<Fn>()))>::result_type, TResults, Fn, TWorkFn, TApplyFn>
    gather(Fn fn) {
        using Traits = function_traits<decltype(std::function(fn))>;
        using NewGathered = typename Traits::result_type;
        return AsyncSystemBuilder<TQueryArgs, NewGathered, TResults, Fn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_source_, fn, work_fn_, apply_fn_
        );
    }

    // (Required) Provide the worker function.
    template<typename Fn>
    AsyncSystemBuilder<TQueryArgs, TGathered, typename function_traits<decltype(std::function(std::declval<Fn>()))>::result_type, TGatherFn, Fn, TApplyFn>
    work(Fn fn) {
        using Traits = function_traits<decltype(std::function(fn))>;
        using NewResults = typename Traits::result_type;
        return AsyncSystemBuilder<TQueryArgs, TGathered, NewResults, TGatherFn, Fn, TApplyFn>(
            world, system_name_prefix, tick_source_, gather_fn_, work_fn_, fn
        );
    }

    // (Optional) Provide a custom apply function.
    template<typename Fn>
    AsyncSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, Fn>
    apply(Fn fn) {
        return AsyncSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, Fn>(
            world, system_name_prefix, tick_source_, gather_fn_, work_fn_, fn
        );
    }

    // Build and register the flecs system.
    void build() {
        build_system(TQueryArgs{});
    }

private:
    template<typename... Comps>
    void build_system(type_list<Comps...>) {
        //std::cout << std::format("Registering async system '{}'", system_name_prefix) << std::endl;
        const std::string future_name = system_name_prefix + "_Future";
        flecs::entity future_component = world.component<Async::Future<TResults>>(future_name.c_str());
        //std::cout << std::format("Future component '{}' registered", future_name) << std::endl;

        // System 1: Kicks off the async task.
        // It queries, applies the gather function, and kicks off the future
        // but without an existing future, to prevent re-launching a task.
        const std::string start_name = std::format("{}_Start", system_name_prefix);
        flecs::system start_sys = world.system<Comps...>(start_name)
            .without(future_component)
            .each([
                ecs = &world,
                gather_fn = gather_fn_,
                work_fn = work_fn_,
                apply_fn = apply_fn_
            ](flecs::entity e, Comps... comps) {
                ZoneScoped; ZoneName(start_name.c_str(), start_name.length());
                
                TGathered gathered_data = gather_fn(e, comps...);
                //std::cout << std::format("Gathered data for async task {}", system_name_prefix) << std::endl;

                // Launch the async task
                std::future<TResults> future = std::async(std::launch::async,
                    [=] {
                        return std::apply(work_fn, gathered_data);
                    });
                
                e.set<Async::Future<TResults>>(std::move(future));
            });

        // System 2: Polls for completed futures and applies the results.
        const std::string check_name = std::format("{}_Check", system_name_prefix);
        flecs::check_sys = world.system<Async::Future<TResults>>(check_name.c_str())
            .each([=](flecs::entity e, Async::Future<TResults>& fut_comp) {
                ZoneScoped; ZoneName(check_name.c_str(), check_name.length());
                //std::cout << std::format("Checking async task for entity {} {}", std::string(e.path()), check_name) << std::endl;
                if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    //std::cout << "Async task completed, retrieving results." << std::endl;
                    TResults results = fut_comp.future.get();
                    //std::cout << "Results retrieved from async task." << std::endl;

                    // Use the results function
                    // We use std:;apply to unpack the result, and to provide the ECS & entity to the apply function
                    std::tuple_cat_t<std::tuple<flecs::world&, flecs::entity>, TResults> apply_args = std::tuple_cat(
                        std::make_tuple(std::ref(*ecs), e),
                        results
                        );
                    std::apply(apply_fn, apply_args);

                    // The task is complete, so remove the trigger and future components.
                    // The slightly not great performance of removing components is ok
                    // here since by definition this should be a low frequency operation
                    e.remove(future_component);
                }
            });

        if (tick_source_.is_valid()) {
            start_sys.tick_source(tick_source_);
            check_sys.tick_source(tick_source_);
        }
    }

    flecs::world& world;
    std::string system_name_prefix;
    flecs::entity tick_source_;
    TGatherFn gather_fn_;
    TWorkFn work_fn_;
    TApplyFn apply_fn_;
};

}