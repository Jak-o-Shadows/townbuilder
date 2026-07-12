#include <tracy/Tracy.hpp>

#include "taskflow_scheduler/module.hpp"

#include "msgLogging/module.hpp"

#include <taskflow/taskflow.hpp>

#include <iostream>
#include <memory>
#include <sstream>
#include <format>

namespace TaskflowScheduler {

std::shared_ptr<spdlog::logger> systemsLogger;


// Configure how many worker stages you want (usually number of hardware threads)
//  TODO: Figure out what thsi actually does
static constexpr int WORKER_STAGE_COUNT = 1;
tf::Executor executor;


static flecs::query<> system_query;

struct SystemInfo {
    flecs:: entity system_entity;
    const ecs_system_t *sys;  // Pointer to the internal C system data
    flecs::entity_t phase;  // The phase this system is in (if any) (zero if none)
};


// Resolve the Phase entity for `entity` by walking DependsOn chain.
// Returns 0 if no Phase found.
ecs_entity_t resolve_phase_cascade(flecs::entity sys_ent) {
    // Try direct DependsOn -> Phase target (C++ API)
    flecs::entity phase_ent = sys_ent.target(flecs::DependsOn, flecs::Phase);
    if (phase_ent.is_valid()) return phase_ent.id();

    // Walk DependsOn chain: follow DependsOn targets until we find a Phase
    /*
    flecs::entity cur = sys_ent;
    while (true) {
        flecs::entity parent = cur.target(flecs::DependsOn); // first DependsOn target
        if (!parent.is_valid()) break;

        phase_ent = parent.target(flecs::DependsOn, flecs::Phase);
        if (phase_ent.is_valid()) return phase_ent.id();

        cur = parent;
    }
        */

    return 0;
}

std::string get_component_str(flecs::world& ecs, const ecs_id_t term_id) {
    char *id_str = ecs_id_str(ecs.c_ptr(), term_id);
    std::string comp = id_str ? id_str : "<unknown>";
    if (id_str) {
        ecs_os_free(id_str);
    }
    return comp;
}


//#[[Phase-grouped dependency graph builder, IMPL_GRAPH_BUILDER, impl, [SPEC_GRAPH_BUILDER]]]
void build_taskflow_graph(flecs::world& ecs) {
    ZoneScoped;
    systemsLogger->debug("Building taskflow graph (in func) ");

    // Round-robin stage assignment counter (thread-safe)
    std::atomic<int> stage_assign_counter{0};  // TODO: Understand this at all


    // Collect systems in scheduler order
    //  Using the query gives us the same order in flecs, which helps make this more
    //  drop-in replacement
    std::vector<SystemInfo> systems;
    systems.reserve(128);  // This is a reasonable upper guess?  // TODO: Consider this more

    systemsLogger->trace("Collecting systems for taskflow graph");
    system_query.each([&systems, &ecs](flecs::entity system_entity) {
        ecs_entity_t id = system_entity.id();
        const ecs_system_t *s = ecs_system_get(ecs.c_ptr(), id);
        ecs_entity_t phase = resolve_phase_cascade(system_entity);
        systems.push_back({system_entity, s, phase});
    });
    systemsLogger->debug("Collected {} systems for taskflow graph", systems.size());
    
    systemsLogger->trace("Grouping systems by phase");
    // Group systems by phase. This is required in order to operate on deferred data appropriately
    std::unordered_map<ecs_entity_t, std::vector<size_t>> systems_by_phase;
    std::vector<ecs_entity_t> phase_order;  // To maintain the order of phases
    for (size_t system_idx = 0; system_idx < systems.size(); system_idx++) {
        const SystemInfo& sys_info = systems[system_idx];
        ecs_entity_t phase = sys_info.phase;
        // Add it to the phase order if it isnt'already there. Because we are iterating in order
        //  this will maintain the order of phases as they appear in the systems list
        if (!systems_by_phase.contains(phase)) {
            phase_order.push_back(phase);
        }
        // Add the system index to the appropriate phase group
        systems_by_phase[phase].push_back(system_idx);
    }
    systemsLogger->trace("Grouped {} systems into {} phases", systems.size(), phase_order.size());

    // Print em
    for (ecs_entity_t phase : phase_order) {
        std::ostringstream msg;
        msg << "Phase: " << (phase ? ecs_get_name(ecs.c_ptr(), phase) : "<no phase>") << " : ";
        for (size_t system_idx : systems_by_phase[phase]) {
            const SystemInfo& sys_info = systems[system_idx];
            msg << sys_info.system_entity.path().c_str() << " , ";
        }
        msg << "END";
        systemsLogger->debug("{}", msg.str());
    }
    systemsLogger->debug("Displayed {} systems in {} phases", systems.size(), phase_order.size());

    systemsLogger->debug("Building taskflow graph for each of {} phases", phase_order.size());
    // Create a taskflow task for each system (maintaining order!)
    for (ecs_entity_t phase : phase_order) {

        ecs.defer_begin();

        systemsLogger->trace("Building taskflow graph for phase: {}", (phase ? ecs_get_name(ecs.c_ptr(), phase) : "<no phase>"));
        tf::Taskflow taskflow;

        std::vector<tf::Task> tasks(systems.size());  // This is the upper bound - probably less

        // Create tasks for each system in this phase
        for (size_t system_idx : systems_by_phase[phase]) {
            // Create a task for this system
            const SystemInfo& sys_info = systems[system_idx];
            systemsLogger->trace("Creating task for system: {} in phase: {}", sys_info.system_entity.path().c_str(), (phase ? ecs_get_name(ecs.c_ptr(), phase) : "<no phase>"));

            
            ecs_entity_t sys_id = sys_info.system_entity.id();

            // Assign a stage to this task (round-robin)
            int stage_id = stage_assign_counter.fetch_add(1) % WORKER_STAGE_COUNT;
            // Get the stage world wrapper and C pointer
            flecs::world stage_world = ecs.get_stage(stage_id);
            ecs_world_t *stage_w = stage_world.c_ptr();

            int stage_current = stage_id;
            int stage_count = WORKER_STAGE_COUNT;  // TODO: Understand thsi

            // Create the task: capture stage_w and sys_id by value
            float delta_time = 1;  // TODO: figure out what this does, and what to actually do
            systemsLogger->trace("Emplacing task for {}", std::string(sys_info.system_entity.path()));
            tasks[system_idx] = taskflow.emplace([stage_w, &ecs, sys_id, delta_time]() {
                systemsLogger->trace("Running system {}", std::string(ecs.entity(sys_id).path()));
                ecs_run(stage_w, sys_id, delta_time, nullptr);
                systemsLogger->trace("Ran system {}", std::string(ecs.entity(sys_id).path()));
            }).name(sys_info.system_entity.path().c_str()); // optional: name task for debugging

            
            systemsLogger->trace("Created task for system: {} in phase: {}", sys_info.system_entity.path().c_str(), (phase ? ecs_get_name(ecs.c_ptr(), phase) : "<no phase>"));
        }

        
        // Dependencies based n terms
        std::unordered_map<ecs_entity_t, size_t> last_writer;  // Map from component id to last writing system index
        std::unordered_map<ecs_entity_t, std::vector<size_t>> active_readers;  // Map from component id to list of reading system indices

        for (const size_t system_idx : systems_by_phase[phase]) {
            const SystemInfo & sys_info = systems[system_idx];
            systemsLogger->trace("Processing dependencies for {}", std::string(sys_info.system_entity.path()));
            const ecs_system_t *s = sys_info.sys;
            if (!s || !s->query) {
                continue;  // Skip if no system data or query // TODO: Is this a sensible thing to do?
            }

            systemsLogger->trace("System {} has {} terms", std::string(sys_info.system_entity.path()), s->query->term_count);
            for (int term_index = 0; term_index < s->query->term_count; term_index++) {
                const ecs_term_t *term = &s->query->terms[term_index];
                const ecs_entity_t comp_id = term->id;
                const std::string comp_str = get_component_str(ecs, comp_id);
                systemsLogger->trace("Processing {}: term {}", std::string(sys_info.system_entity.path()), comp_str);
                const ecs_inout_kind_t access = static_cast<ecs_inout_kind_t>(term->inout);

                // Read
                switch (access) {
                    case EcsIn:
                        systemsLogger->trace("In");
                        // Look for a previous writer
                        if (last_writer.contains(comp_id)) {
                            tasks[last_writer[comp_id]].precede(tasks[system_idx]);
                        }
                        // Add this system to the active readers for this component
                        active_readers[comp_id].push_back(system_idx);
                        break;
                    case EcsInOut:
                        // Deliberately fallthrough
                        systemsLogger->trace("InOut");
                    case EcsOut:
                        systemsLogger->trace("Out (or InOut)");
                        // Look for a previous writer
                        if (last_writer.contains(comp_id)) {
                            tasks[last_writer[comp_id]].precede(tasks[system_idx]);
                        }
                        // Look for active readers and add dependencies
                        if (active_readers.contains(comp_id)) {
                            for (size_t reader_idx : active_readers[comp_id]) {
                                tasks[reader_idx].precede(tasks[system_idx]);
                            }
                            active_readers[comp_id].clear();  // Clear the list of active readers for this component
                        }
                        // Update the last writer for this component
                        last_writer[comp_id] = system_idx;
                        break;
                    default:
                        systemsLogger->trace("default");
                        // Do nothing for other access types
                        // TODO: Log something to understand this
                        break;
                }
            }
            systemsLogger->trace("Processed all dependencies for system {}", std::string(sys_info.system_entity.path()));
        }
        systemsLogger->trace("Finished processing all systems for phase {}", phase);

        systemsLogger->debug("Graph is:");
        taskflow.dump(std::cout);

        systemsLogger->trace("Running taskflow for phase {}", phase);
        executor.run(taskflow).wait();  // Wait for all tasks in this phase to complete before moving to the next phase
        systemsLogger->debug("Finished running taskflow for phase {}", phase);

        // Merge
        systemsLogger->trace("About to end deferring: operations for phase {}", phase);
        ecs.defer_end();  // Merge deferred operations from all stages back to the main world
        systemsLogger->debug("Deferring ended: operations for phase {}", phase);
        
    }

}





systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    ecs.import<TaskflowScheduler::components>();

    systemsLogger->trace("Other flecs modules imported");
    
    system_query =  ecs.query_builder()
        .with(flecs::System)
        .with(flecs::Phase).cascade(flecs::DependsOn)
        .without(flecs::Disabled).up(flecs::DependsOn)
        .without(flecs::Disabled).up(flecs::ChildOf)
        .without<TaskflowExempt>()
        .build();
    systemsLogger->debug("Built query for systems");

    ecs.system("SystemLister")
        .run([](flecs::iter& it) {
            ZoneScopedN("SystemLister");
            systemsLogger->debug("Listing all systems:");
            std::ostringstream msg;
            msg << "Systems: ";
            system_query.each([&msg](flecs::entity e) {
                msg << e.path().c_str() << "-> ";
            });
            msg << "END FRAME";
            systemsLogger->debug("{}", msg.str());
        });

    ecs.system("SystemInOutLister")
        .run([&ecs](flecs::iter& it) {
            ZoneScopedN("SystemInOutLister");
            systemsLogger->debug("Listing all systems in & outs:");

            system_query.each([&ecs](flecs::entity system_entity) {
                ecs_entity_t id = system_entity.id();
                const ecs_system_t *s = ecs_system_get(ecs.c_ptr(), id);
                if (!s || !s->query) {
                    systemsLogger->debug("{} : <no system data>", system_entity.path().c_str());
                    return;
                }
                std::ostringstream msg;
                msg << std::format("System: {} : ", system_entity.path().c_str());
                systemsLogger->trace("System: {}", std::string(system_entity.path()));
                for (int t = 0; t < s->query->term_count; ++t) {
                    const ecs_term_t *term = &s->query->terms[t];
                    systemsLogger->trace("Component id: {}", term->id);
                    // term->id is the component id; term->inout is the access kind
                    std::string comp = get_component_str(ecs, term->id);

                    const char *access = "unknown";
                    switch (term->inout) {
                        case EcsIn: access = "in"; break;
                        case EcsOut: access = "out"; break;
                        case EcsInOut: access = "inout"; break;
                        default: access = "other"; break;
                    }
                    systemsLogger->trace("{}({}) ;", comp, access);
                    msg << std::format("{}({}) ;", comp, access);
                }
                systemsLogger->debug("{} END", msg.str());
            });
        });

    ecs.system("TaskflowGraphBuilder")
        .kind(flecs::OnStart)
        .run([&ecs](flecs::iter& it) {
            ZoneScopedN("TaskflowGraphBuilder");
            systemsLogger->debug("Building taskflow graph");
            build_taskflow_graph(ecs);
        })
        .add(flecs::Disabled);

}

}