Taskflow — Plan
===============

Specifications for :need:`FEAT_TASKFLOW_PARALLELISM`

.. spec:: Phase-grouped Dependency Graph Builder
   :id: SPEC_GRAPH_BUILDER
   :specified_by: REQ_TASKFLOW_SCHEDULER, REQ_ANNOTATION_MAPPING
   :status: done

   All registered Flecs systems are collected via a query ordered by phase (using ``cascade(flecs::DependsOn)``). Systems are grouped into per-phase buckets. For each phase a fresh ``tf::Taskflow`` is constructed; each system becomes a task that calls ``ecs_run()`` on a stage-local world pointer. Dependency edges are added by scanning each system's ``ecs_term_t`` array:

   - **EcsIn**: add edge from the last writer of that component to this task; register this task as an active reader.
   - **EcsOut / EcsInOut**: add edges from all active readers and from the last writer of that component to this task; update the last writer; clear active readers.

   After all tasks are wired, the phase taskflow is submitted to a module-level ``tf::Executor`` and awaited before the next phase begins. Deferred ECS operations are merged with ``ecs.defer_end()`` after each phase.

.. spec:: TaskflowExempt Tag
   :id: SPEC_TASKFLOW_EXEMPT
   :specified_by: REQ_TASKFLOW_EXEMPT
   :status: done

   The ``TaskflowExempt`` tag is declared in ``module.hpp``. The system query that feeds ``build_taskflow_graph()`` explicitly excludes entities carrying this tag via ``.without<TaskflowExempt>()``.

.. spec:: Tracy Profiler Integration
   :id: SPEC_TRACY_INTEGRATION
   :specified_by: REQ_TRACY_PROFILING
   :status: done

   ``ZoneScoped`` / ``ZoneScopedN`` Tracy macros are placed around ``build_taskflow_graph`` and around each system callback in the demonstrator. The ``TRACY_ENABLE`` preprocessor guard controls inclusion of the Tracy header in ``scheduler.cpp``.

.. spec:: Standalone Demonstrator
   :id: SPEC_DEMONSTRATOR
   :specified_by: REQ_TASKFLOW_BUILD_TARGET
   :status: done

   ``src/taskflow_scheduler/demonstrator/`` is an independent CMake project with its own ``conanfile.py``. It imports the ``TaskflowScheduler`` module, registers synthetic producer and consumer systems for ``DataComponentA`` and ``DataComponentB`` with artificial ``Sleep()`` delays, then drives scheduling by calling ``build_taskflow_graph()`` directly in a bare ``while(true)`` loop.

.. spec:: Main Application Integration
   :id: SPEC_MAIN_APP_INTEGRATION
   :specified_by: REQ_MAIN_APP_INTEGRATION
   :status: draft

   The ``TaskflowGraphBuilder`` Flecs system (registered in ``systems.cpp`` on ``flecs::OnStart``) is currently marked ``.add(flecs::Disabled)``. Integration requires deciding on a per-frame invocation strategy (override ``ecs.progress()`` vs. calling ``build_taskflow_graph()`` each frame as a running system), resolving the ``delta_time`` placeholder (currently hardcoded to ``1``), and deciding on the worker-stage count (``WORKER_STAGE_COUNT`` is currently ``1``).
