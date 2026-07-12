PLUGIN SYSTEM — V&V
===================

Validation for :need:`FEAT_PLUGIN_SYSTEM`

.. test:: Plugin Load Failure Test
    :id: TC_PLUGIN_LOAD_FAIL
    :test_type: automated
    :verified_by: SPEC_PLUGIN_LOADER
    :status: draft

    Verify that :code:`PluginLoader` throws a clear :code:`std::runtime_error` when failing to load a library or resolve symbols.

.. test:: Plugin Tick Execution Test
    :id: TC_PLUGIN_TICK
    :test_type: automated
    :verified_by: SPEC_PLUGIN_LOADER
    :status: draft

    Ensure :code:`plugin_tick` correctly invokes the external function with appropriate :code:`GUID` and :code:`TickInput`.

.. test:: Library Unloading Test
    :id: TC_LIBRARY_UNLOAD
    :test_type: automated
    :verified_by: SPEC_LIFETIME
    :status: draft

    Verify that library handles are correctly freed upon :code:`PluginLoader` destruction.

