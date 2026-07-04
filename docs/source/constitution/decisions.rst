Decisions
=========

Key architectural and technology decisions for AgentClinic. Each decision is
tracked as a ``.. decision::`` directive with a unique ID.

.. needtable::
   :types: decision
   :style: table
   :columns: id;title;status

Language & Runtime
------------------

.. decision:: Use Flecs
    :id: DECISION_USE_FLECS
    :status: accepted
    :author: Jak-o-Shadows

    The Flecs ECS framework (https://www.flecs.dev/) will be used as the primary framework for the game.

.. decision:: Create external data visualisation tool in Python
    :id: DECISION_CREATE_EXTERNAL_DATA_VISUALISATION_TOOL_IN_PYTHON
    :status: accepted

    Create a Python tool to visualise the data from the game, using the Flecs Python binding. This will allow for rapid prototyping and testing of new features, as well as providing a way to visualise the data in a more user-friendly way. This is quicker than doing it in-engine.

.. decision:: Use Conan for C++ dependency management
    :id: DECISION_USE_CONAN_FOR_CPP_DEPENDENCY_MAN
    :status: accepted

    Use Conan (https://conan.io/) for C++ dependency management. It is a popular and well-supported package manager for C++ that works well with CMake, and is used by many large projects. It makes it easier to get new libraries and keep them up to date.

Testing
-------

.. decision:: Use Catch2 for C++ unit testing
    :id: DECISION_USE_CATCH2_FOR_CPP_UNIT_TESTING
    :status: accepted

    Use Catch2 (https://github.com/catchorg/Catch2) for C++ unit testing. It is used by Unreal Engine, and is available on Conan.

