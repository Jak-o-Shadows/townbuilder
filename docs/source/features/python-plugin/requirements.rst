Python Plugin System — Requirements
===================================

Requirements for :need:`FEAT_PYTHON_PLUGIN_SYSTEM`

.. req:: Python Bindings
    :id: REQ_PYTHON_BINDINGS
    :requires: FEAT_PYTHON_PLUGIN_SYSTEM
    :status: open

    The plugin system must expose its components and loading mechanisms to Python via :code:`pybind11` to facilitate testing, data manipulation, and scripting.

.. req:: Automatic ECS Component Binding
    :id: REQ_ECS_COMPONENT_PYTHON_BINDING
    :requires: REQ_PYTHON_BINDINGS
    :status: open

    Reuse the Flecs component reflection data to avoid duplication of the type data

.. req:: Numpy Integration
    :id: REQ_NUMPY_INTEGRATION
    :requires: FEAT_PYTHON_PLUGIN_SYSTEM
    :status: open

    Plugins must support high-performance data exchange with Python using :code:`numpy` arrays


