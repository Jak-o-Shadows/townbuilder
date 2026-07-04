Taskflow — Validation
=====================

Test cases for :need:`FEAT_TASKFLOW_PARALLELISM`

Automated
---------

.. test:: Dependency Resolution Verification
   :id: TC_DEPENDENCY_RESOLVE
   :test_type: automated
   :verified_by: SPEC_GRAPH_BUILDER
   :status: draft

   Unit tests should verify that each registered system becomes a Taskflow task and that dependency edges are created from the declared access patterns (.in, .out, .inout) in a way that preserves ordering for conflicting resource access.

.. test:: Drop-in Wrapper Verification
   :id: TC_DROP_IN_WRAPPER
   :test_type: automated
   :verified_by: SPEC_SYSTEM_WRAPPER
   :status: draft

   Verify that existing systems can be declared with the normal Flecs syntax and are executed through the Taskflow wrapper without requiring changes to the system declaration code.

.. test:: Standalone Build Target Verification
   :id: TC_BUILD_TARGET
   :test_type: automated
   :verified_by: SPEC_SYSTEM_WRAPPER
   :status: draft

   Verify that the standalone build target correctly links Taskflow and compiles the scheduler.

Manual
------

.. test:: Tracy Instrumentation Verification
   :id: TC_TRACY_LOGGING
   :test_type: manual
   :verified_by: SPEC_TRACY_INTEGRATION
   :status: draft

   Inspect Tracy profiler GUI output to confirm task execution spans are logged correctly.
