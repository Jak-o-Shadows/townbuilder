Taskflow — Plan
===============

Specifications for :need:`FEAT_TASKFLOW_PARALLELISM`

.. spec:: Dependency Graph Builder
   :id: SPEC_GRAPH_BUILDER
   :specified_by: REQ_ANNOTATION_MAPPING
   :status: draft

   Implement a scheduler component that inspects each registered Flecs system, assigns it to its own Taskflow task, and builds dependency edges from the declared access annotations (.in, .out, .inout). The dependency logic should follow a conservative resource-based model so that read-after-write and write-after-read/write conflicts serialize correctly.

.. spec:: Drop-in System Wrapper
   :id: SPEC_SYSTEM_WRAPPER
   :specified_by: REQ_TASKFLOW_SCHEDULER, REQ_DROP_IN_WRAPPER, REQ_TASKFLOW_BUILD_TARGET
   :status: draft

   Develop a wrapper that can replace the normal per-frame Flecs execution step without changing the declaration style of existing systems. The wrapper should preserve the existing ``ecs.system(...)`` definition pattern and delegate execution to Taskflow while managing the executor lifetime.

.. spec:: Tracy Profiler Implementation
   :id: SPEC_TRACY_INTEGRATION
   :specified_by: REQ_TRACY_PROFILING
   :status: draft

   Embed Tracy profiling macros around each system task so that individual system execution spans are visible in the profiler output.

.. spec:: Flecs Graph Reuse Consideration
   :id: SPEC_FLECS_GRAPH_REUSE
   :specified_by: REQ_ANNOTATION_MAPPING
   :status: draft

   The implementation should prefer a lightweight, local dependency builder over direct reuse of Flecs' internal graph structure, because Flecs' internal scheduling machinery is not expected to be straightforward to reuse from the C++ wrapper layer.

