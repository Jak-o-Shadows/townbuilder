ImGui Log Sink — Plan
===========================

Implementation plan for :need:`FEAT_IMGUI_LOG_SINK`

.. spec:: spdlog Sink Implementation
   :id: SPEC_LOG_SINK_CLASS
   :specified_by: REQ_LOG_SINK_DISPLAY
   :status: draft

   Create a custom :code:`spdlog::sinks::base_sink` that writes log messages into a thread-safe internal buffer.

.. spec:: ImGui Window Integration
   :id: SPEC_LOG_SINK_IMGUI
   :specified_by: REQ_LOG_SINK_FILTER
   :status: draft

   Implement an ImGui rendering loop that draws the internal buffer content, including UI controls for log level filtering.
