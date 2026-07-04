Taskflow — Requirements
=======================

Requirements for :need:`FEAT_TASKFLOW_PARALLELISM`

.. req:: System Scheduler Integration
   :id: REQ_TASKFLOW_SCHEDULER
   :requires: FEAT_TASKFLOW_PARALLELISM
   :priority: high
   :status: open

   Implement a scheduler backend that replaces the default Flecs system execution step with a Taskflow-backed executor while preserving the existing Flecs system definition syntax.

.. req:: Flecs Annotation Dependency Mapping
   :id: REQ_ANNOTATION_MAPPING
   :requires: FEAT_TASKFLOW_PARALLELISM
   :priority: high
   :status: open

   Treat each registered Flecs system as a separate Taskflow task and derive dependencies from the system access annotations (.in, .out, .inout). A system that reads a resource must wait for prior writers, and a system that writes a resource must wait for prior readers/writers that touched the same resource.

.. req:: Drop-in Execution Wrapper
   :id: REQ_DROP_IN_WRAPPER
   :requires: FEAT_TASKFLOW_PARALLELISM
   :priority: high
   :status: open

   Provide a wrapper that can be substituted for the normal per-frame Flecs execution path without requiring any changes to how systems are declared. Existing code should continue to use the standard ``ecs.system(...).term_at(...).in/out/inout().each(...)`` pattern.

.. req:: Taskflow Standalone Build Target
   :id: REQ_TASKFLOW_BUILD_TARGET
   :requires: FEAT_TASKFLOW_PARALLELISM
   :priority: medium
   :status: open

   Provide a standalone build target that demonstrates and tests the Taskflow-backed scheduler in isolation from the main application.

.. req:: Tracy Performance Instrumentation
   :id: REQ_TRACY_PROFILING
   :requires: FEAT_TASKFLOW_PARALLELISM
   :priority: medium
   :status: open

   Integrate Tracy performance logging for monitoring per-system task execution spans.