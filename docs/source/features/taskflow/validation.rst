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

   Unit tests should verify that, given a set of systems with known ``.in``/``.out``/``.inout`` annotations, ``build_taskflow_graph()`` produces the correct dependency edges: readers wait on prior writers; writers wait on prior readers and prior writers of the same component.

.. test:: TaskflowExempt Exclusion Verification
   :id: TC_TASKFLOW_EXEMPT
   :test_type: automated
   :verified_by: SPEC_TASKFLOW_EXEMPT
   :status: draft

   Verify that a system entity tagged with ``TaskflowExempt`` is not included in the taskflow graph constructed by ``build_taskflow_graph()``.

.. test:: Standalone Demonstrator Build and Run
   :id: TC_BUILD_TARGET
   :test_type: automated
   :verified_by: SPEC_DEMONSTRATOR
   :status: done

   The ``demonstrator/`` target builds and runs without error. Correct ordering (consumers after producers) can be confirmed from log output.

Manual
------

.. test:: Tracy Instrumentation Verification
   :id: TC_TRACY_LOGGING
   :test_type: manual
   :verified_by: SPEC_TRACY_INTEGRATION
   :status: done

   Attach the Tracy profiler to the running demonstrator and confirm that task execution spans for each system are logged correctly and that parallel tasks appear overlapping on the timeline.

.. test:: Phase Ordering Verification
   :id: TC_PHASE_ORDERING
   :test_type: manual
   :verified_by: SPEC_GRAPH_BUILDER
   :status: draft

   Run the demonstrator and confirm via log output that systems in different Flecs phases execute strictly in phase order, with deferred operations merged between phases.
