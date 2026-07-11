Taskflow — Requirements
=======================

Requirements for :need:`FEAT_TASKFLOW_PARALLELISM`

.. req:: Dependency Graph Construction
   :id: REQ_TASKFLOW_SCHEDULER
   :requires: FEAT_TASKFLOW_PARALLELISM
   :status: done

   Build a Taskflow dependency graph from all registered Flecs systems each frame. Systems must be grouped by Flecs phase, and dependency edges between tasks must be derived from each system's declared access annotations (``.in``, ``.out``, ``.inout``) so that read-after-write and write-after-read/write conflicts are serialised correctly. Systems must run phase-by-phase, with deferred ECS operations merged between phases.

.. req:: Flecs Annotation Dependency Mapping
   :id: REQ_ANNOTATION_MAPPING
   :requires: FEAT_TASKFLOW_PARALLELISM
   :status: done

   Treat each registered Flecs system as a separate Taskflow task and derive dependencies from the system access annotations (``.in``, ``.out``, ``.inout``). A system that reads a resource must wait for prior writers; a system that writes a resource must wait for prior readers and writers that touched the same resource.

.. req:: TaskflowExempt Opt-Out Tag
   :id: REQ_TASKFLOW_EXEMPT
   :requires: FEAT_TASKFLOW_PARALLELISM
   :status: done

   Provide a ``TaskflowExempt`` tag component that can be added to any Flecs system entity to exclude it from taskflow graph scheduling.

.. req:: Taskflow Standalone Demonstrator
   :id: REQ_TASKFLOW_BUILD_TARGET
   :requires: FEAT_TASKFLOW_PARALLELISM
   :status: done

   Provide a standalone build target (``demonstrator/``) that demonstrates and validates the Taskflow-backed scheduler in isolation from the main application, using synthetic producer/consumer systems with artificial sleep delays to make parallelism observable.

.. req:: Tracy Performance Instrumentation
   :id: REQ_TRACY_PROFILING
   :requires: FEAT_TASKFLOW_PARALLELISM
   :status: done

   Integrate Tracy performance logging so that individual system execution spans are visible in the Tracy profiler GUI.

.. req:: Main Application Integration
   :id: REQ_MAIN_APP_INTEGRATION
   :requires: FEAT_TASKFLOW_PARALLELISM
   :status: open

   Integrate the Taskflow scheduler into the main application as a replacement for the normal per-frame ``ecs.progress()`` call, without requiring changes to how existing systems are declared.