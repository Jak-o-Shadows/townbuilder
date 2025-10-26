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

// Helper to decay all types in a tuple
template<typename Tup> struct decay_tuple;
template<typename... Ts> struct decay_tuple<std::tuple<Ts...>> {
    using type = std::tuple<std::decay_t<Ts>...>;
};

 
// Helper to get traits from any callable type (lambda, function pointer, std::function)
template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};
 
template<typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {
    using result_type = ReturnType;
    using arg_tuple = std::tuple<Args...>;
};
 
template<typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...)> {
    using result_type = ReturnType;
    using arg_tuple = std::tuple<Args...>;
};

template<typename ResultType>
struct Future {
    std::future<ResultType> future;
};


// Forward declarations for the builders
template <
    typename TQueryArgs,  // type_list of components to query for
    typename TGathered,   // A std::tuple<> of the data prepared by the gather stage
    typename TResults,    // A std::tuple<> of the results from the work stage
    typename TGatherFn,   // The gather function type
    typename TWorkFn,     // The work function type
    typename TApplyFn     // The apply function type
>
class AsyncSystemBuilder;

template <
    typename TQueryArgs,  // type_list of components to query for
    typename TGathered,   // A std::tuple<> of the data prepared by the gather stage
    typename TResults,    // A std::tuple<> of the results from the work stage
    typename TGatherFn,   // The gather function type
    typename TWorkFn,     // The work function type
    typename TApplyFn     // The apply function type
>
class AsyncSingletonSystemBuilder;



// Default Gather: Copy input components
struct DefaultEntityGatherFn {
    template<typename... Args>
    std::tuple<std::decay_t<Args>...> operator()(flecs::iter& it, size_t i, const Args&... args) const {
        // The default gather copies the queried components for the current entity.
        return std::make_tuple(std::decay_t<Args>(args)...);
    }
};

// Default Apply: Set result components on the entity.
// Handles both tags (using add) and components with data (using set).
struct DefaultEntityApplyFn {
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
    template<typename... TResults>
    void operator()(flecs::iter& it, size_t i, const TResults&... results) const {
        (apply_result(it.entity(i), results), ...);
    }    
};

// Default Gather for singleton systems: Copy input singleton components.
struct DefaultSingletonGatherFn {
    template<typename... Args>
    std::tuple<std::decay_t<Args>...> operator()(flecs::world&, const Args&... args) const {
        return std::make_tuple(std::decay_t<Args>(args)...);
    }
};

// Default Apply for singleton systems: Set result components as singletons.
struct DefaultSingletonApplyFn {
private:
    template<typename T>
    void apply_result(flecs::world& world, const T& result) const {
        if constexpr (std::is_empty_v<T>) {
            world.add<T>();
        } else {
            world.set<T>(result);
        }
    }
public:
    template<typename... TResults>
    void operator()(flecs::world& world, const TResults&... results) const {
        (apply_result(world, results), ...);
    }    
};

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
 *    As it is called from a regular system, it is inherently thread safe. Gets returned data from work.
 *    By default, it adds/sets any returned values as components to the entity.
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
    AsyncSystemBuilder(flecs::world& ecs, const std::string name, flecs::entity tick_src,
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
    gather(Fn gather_fn_new) {
        using NewGathered = typename function_traits<Fn>::result_type;
        return AsyncSystemBuilder<TQueryArgs, NewGathered, TResults, Fn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_source_, gather_fn_new, work_fn_, apply_fn_
        );
    }

    // (Required) Provide the worker function.
    template<typename Fn>
    auto
    work(Fn fn) {
        using WorkerArgTuple = typename function_traits<Fn>::arg_tuple;
        using NewGathered = typename decay_tuple<WorkerArgTuple>::type;
        using NewResults = typename function_traits<Fn>::result_type;
        return AsyncSystemBuilder<TQueryArgs, NewGathered, NewResults, DefaultEntityGatherFn, Fn, TApplyFn>(
            world, system_name_prefix, tick_source_, DefaultEntityGatherFn{}, fn, DefaultEntityApplyFn{}
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
        flecs::system start_sys = world.system<Comps...>(start_name.c_str())
            .without(future_component)
            .each([
                &world = world,
                start_name = start_name,
                system_name_prefix = system_name_prefix,
                gather_fn = gather_fn_,
                work_fn = work_fn_
            ](flecs::iter& it, size_t i, Comps... comps) {
                ZoneScoped; ZoneName(start_name.c_str(), start_name.length());
                
                TGathered gathered_data = gather_fn(it, i, comps...);
                //std::cout << std::format("Gathered data for async task {}", system_name_prefix) << std::endl;
                flecs::entity e = it.entity(i);

                // Launch the async task
                std::future<TResults> future = std::async(std::launch::async,
                    [work_fn, gathered_data] { // Capture work_fn and gathered_data by value
                        return std::apply(work_fn, gathered_data);
                    });
                
                // Construct the Future component with the std::future
                e.set<Async::Future<TResults>>({std::move(future)});
            });

        // System 2: Polls for completed futures and applies the results.
        const std::string check_name = std::format("{}_Check", system_name_prefix);
        flecs::system check_sys = world.system<Async::Future<TResults>>(check_name.c_str())
            .each([&world = world,  // Capture world by reference
                   apply_fn = apply_fn_,
                   check_name,
                   future_component
                ](flecs::iter& it, size_t i, Async::Future<TResults>& fut_comp) {
                ZoneScoped; ZoneName(check_name.c_str(), check_name.length());
                if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    TResults results = fut_comp.future.get();

                    // Use the results function.
                    // We use std::apply to unpack the result and provide the iter and index to the apply function.
                    auto apply_args = std::tuple_cat(
                        std::make_tuple(std::ref(it), i),
                        results
                    );
                    std::apply(apply_fn, apply_args);

                    // The task is complete, so remove the trigger and future components.
                    // The slightly not great performance of removing components is ok
                    // here since by definition this should be a low frequency operation
                    it.entity(i).remove(future_component);
                }
            });

        // TODO: Put tick source back in
        //  system.tick_source is not a thing, so will have to figure it out later
        //if (tick_source_.is_valid()) {
        //    start_sys.tick_source(tick_source_);
        //    check_sys.tick_source(tick_source_);
        //}
        //std::cout << std::format("Registered async system '{}'", system_name_prefix);
    }

    flecs::world& world;
    std::string system_name_prefix;
    flecs::entity tick_source_;
    TGatherFn gather_fn_;
    TWorkFn work_fn_;
    TApplyFn apply_fn_;
};

/**
 * @brief A builder for creating efficient, thread-safe, asynchronous operations on singleton components.
 *
 * This builder is a specialization for singleton components, following a similar "gather-work-apply" pipeline:
 * 1. GATHER (Main Thread): A `gather` function runs inside a flecs system on the main thread.
 *    It safely extracts data from singleton components. By default, it copies the queried singletons.
 * 2. WORK (Worker Thread): A `work` function runs on a background thread via std::async.
 *    It operates exclusively on the thread-safe data prepared by the `gather` stage.
 * 3. APPLY (Main Thread): An `apply` function runs on the main thread after the work is complete.
 *    By default, it sets any returned values as new singleton components.
 */
template <typename TQueryArgs, typename TGathered, typename TResults, typename TGatherFn, typename TWorkFn, typename TApplyFn>
class AsyncSingletonSystemBuilder {
public:
    AsyncSingletonSystemBuilder(flecs::world& ecs, const std::string name, flecs::entity tick_src,
                       TGatherFn gather_fn, TWorkFn work_fn, TApplyFn apply_fn) 
        : world(ecs), system_name_prefix(name), tick_source_(tick_src),
          gather_fn_(gather_fn), work_fn_(work_fn), apply_fn_(apply_fn) {}

    // (Required) Define the singleton components to query for.
    template<typename... Comps>
    AsyncSingletonSystemBuilder<type_list<Comps...>, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>
    query() {
        return AsyncSingletonSystemBuilder<type_list<Comps...>, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_source_, gather_fn_, work_fn_, apply_fn_
        );
    }

    // Set the system's tick source.
    AsyncSingletonSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>
    tick_source(flecs::entity tick_src) {
        return AsyncSingletonSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_src, gather_fn_, work_fn_, apply_fn_
        );
    }

    // (Optional) Provide a custom gather function.
    template<typename Fn>
    AsyncSingletonSystemBuilder<TQueryArgs, typename function_traits<Fn>::result_type, TResults, Fn, TWorkFn, TApplyFn>
    gather(Fn gather_fn_new) {
        using NewGathered = typename function_traits<Fn>::result_type;
        return AsyncSingletonSystemBuilder<TQueryArgs, NewGathered, TResults, Fn, TWorkFn, TApplyFn>(
            world, system_name_prefix, tick_source_, gather_fn_new, work_fn_, apply_fn_
        );
    }

    // (Required) Provide the worker function.
    template<typename Fn>
    AsyncSingletonSystemBuilder<TQueryArgs, typename decay_tuple<typename function_traits<Fn>::arg_tuple>::type, typename function_traits<Fn>::result_type, DefaultSingletonGatherFn, Fn, DefaultSingletonApplyFn>
    work(Fn fn) {
        using WorkerArgTuple = typename function_traits<Fn>::arg_tuple;
        using NewGathered = typename decay_tuple<WorkerArgTuple>::type;
        using NewResults = typename function_traits<Fn>::result_type;
        return AsyncSingletonSystemBuilder<TQueryArgs, NewGathered, NewResults, DefaultSingletonGatherFn, Fn, DefaultSingletonApplyFn>(
            world, system_name_prefix, tick_source_, DefaultSingletonGatherFn{}, fn, DefaultSingletonApplyFn{}
        );
    }

    // (Optional) Provide a custom apply function.
    template<typename Fn>
    AsyncSingletonSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, Fn>
    apply(Fn fn) {
        return AsyncSingletonSystemBuilder<TQueryArgs, TGathered, TResults, TGatherFn, TWorkFn, Fn>(
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
        const std::string future_name = system_name_prefix + "_Future";
        flecs::entity future_component = world.component<Async::Future<TResults>>(future_name.c_str())
            .add(flecs::Singleton);

        const std::string start_name = std::format("{}_Start", system_name_prefix);
        flecs::system start_sys = world.system<Comps...>(start_name.c_str())
            .without(future_component)
            .each([
                &world = world,
                start_name = start_name,
                system_name_prefix = system_name_prefix,
                gather_fn = gather_fn_,
                work_fn = work_fn_
            ](Comps... comps) {
                ZoneScoped; ZoneName(start_name.c_str(), start_name.length());
                
                TGathered gathered_data = gather_fn(world, comps...);
                //std::cout << std::format("Gathered data for async task {}", system_name_prefix) << std::endl;

                // Launch the async task
                std::future<TResults> future = std::async(std::launch::async,
                    [work_fn, gathered_data] { // Capture work_fn and gathered_data by value
                        return std::apply(work_fn, gathered_data);
                    });
                
                // Construct the Future component with the std::future
                world.set<Async::Future<TResults>>({std::move(future)});
            });

        const std::string check_name = std::format("{}_Check", system_name_prefix);
        flecs::system check_sys = world.system<Async::Future<TResults>>(check_name.c_str())
            .each([&world = world,  // Capture world by reference
                   apply_fn = apply_fn_,
                   check_name,
                   future_component
                ](flecs::iter& it, size_t i, Async::Future<TResults>& fut_comp) {
                ZoneScoped; ZoneName(check_name.c_str(), check_name.length());
                if (fut_comp.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    TResults results = fut_comp.future.get();

                    // Use the results function.
                    // We use std::apply to unpack the result and provide the iter and index to the apply function.
                    auto apply_args = std::tuple_cat(
                        std::make_tuple(std::ref(world)),
                        results
                    );
                    std::apply(apply_fn, apply_args);

                    // The task is complete, so remove the trigger and future components.
                    // The slightly not great performance of removing components is ok
                    // here since by definition this should be a low frequency operation
                    world.remove<Async::Future<TResults>>();  // Unsure why i can't just do world.remove(future_component) like with entities
                }
            });

        if (tick_source_.is_valid()) {
            start_sys.set_tick_source(tick_source_);
            check_sys.set_tick_source(tick_source_);
        }
    }

    flecs::world& world;
    std::string system_name_prefix;
    flecs::entity tick_source_;
    TGatherFn gather_fn_;
    TWorkFn work_fn_;
    TApplyFn apply_fn_;
};





// Entry points: returns a builder with default functions.
inline AsyncSystemBuilder<
    type_list<>, 
    type_list<>, 
    type_list<>, 
    DefaultEntityGatherFn, 
    std::nullptr_t, 
    DefaultEntityApplyFn
>
create_async_system(flecs::world& ecs, const std::string name) {
    return AsyncSystemBuilder<
        type_list<>, 
        type_list<>, 
        type_list<>, 
        DefaultEntityGatherFn, 
        std::nullptr_t, 
        DefaultEntityApplyFn
    >(ecs, name, flecs::entity::null(), DefaultEntityGatherFn{}, nullptr, DefaultEntityApplyFn{});
}
// Entry point for singleton async systems.
inline AsyncSingletonSystemBuilder<
    type_list<>, 
    type_list<>, 
    type_list<>, 
    DefaultSingletonGatherFn, 
    std::nullptr_t, 
    DefaultSingletonApplyFn
>
create_async_system_for_singleton(flecs::world& ecs, const std::string name) {
    return AsyncSingletonSystemBuilder<
        type_list<>, 
        type_list<>, 
        type_list<>, 
        DefaultSingletonGatherFn, 
        std::nullptr_t, 
        DefaultSingletonApplyFn
    >(ecs, name, flecs::entity::null(), DefaultSingletonGatherFn{}, nullptr, DefaultSingletonApplyFn{});
}

}