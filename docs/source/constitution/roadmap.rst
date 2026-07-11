Roadmap
=======

Planned features, in delivery order. Each feature is ``.. feature::`` directive linked to the decisions that constrain it.

.. feature:: System-based paralellism using Taskflow
    :id: FEAT_TASKFLOW_PARALLELISM
    :status: in_progress

    Use Taskflow (https://taskflow.github.io/) to write a scheduler that uses the regular flecs :code:`.in`, :code:`.out`, and :code:`.inout` annotations to automatically parallelize systems, using the annotations to build a dependency graph of systems and then executing them in parallel where possible.

.. feature:: ImGui Log Sink
    :id: FEAT_IMGUI_LOG_SINK
    :status: open

    Implement a custom spdlog sink that pipes log messages into an ImGui-based developer console window, featuring real-time updates and log level filtering.

