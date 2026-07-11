ImGui Log Sink — Validation
=================================

Validation for :need:`FEAT_IMGUI_LOG_SINK`

.. test:: Sink Integration Test
   :id: TC_LOG_SINK_INTEGRATION
   :test_type: automated
   :verified_by: SPEC_LOG_SINK_CLASS
   :status: draft

   Verify that messages sent via spdlog are correctly captured by the custom sink and stored in the internal buffer.

.. test:: Filter Functionality Test
   :id: TC_LOG_SINK_FILTER
   :test_type: manual
   :verified_by: SPEC_LOG_SINK_IMGUI
   :status: draft

   Manually verify that UI toggles correctly hide/show log entries based on their severity level.
