cmake_minimum_required(VERSION 3.26.4)

project(pythonEcsBinding LANGUAGES CXX)

find_package(pybind11 REQUIRED CONFIG)

pybind11_add_module(pythonEcsBinding MODULE binding.cpp)
set(DIR_DEPS ${CMAKE_CURRENT_LIST_DIR}/../../deps)

target_sources(pythonEcsBinding PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/buildings/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/coordinates/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/coordinates/systems.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dis/components.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dis/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/map/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/msgLogging/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/msgLogging/systems.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pathfinding/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pawn/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pawn/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/plugin/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/plugin/systems.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pythonEcsBinding/components.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pythonEcsBinding/systems.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/render/components.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/render/systems.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/render/renderNavmesh.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/render/renderBasics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/statemachine/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/statemachine/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ticks/module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/components.cpp
    ${DIR_DEPS}/flecs_components_cglm.c
    ${DIR_DEPS}/flecs_components_geometry.c
    ${DIR_DEPS}/flecs_components_graphics.c
    ${DIR_DEPS}/flecs_components_gui.c
    ${DIR_DEPS}/flecs_components_physics.c
    ${DIR_DEPS}/flecs_components_transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/simdis/Angle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/simdis/CoordinateConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/simdis/CoordinateSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/simdis/Math.cpp


)

# Set the C++ standard to 20, which is required for some of the template
# features used in the binding code.
set_property(TARGET pythonEcsBinding PROPERTY CXX_STANDARD 20)

# Define _USE_MATH_DEFINES to make constants like M_PI available on Windows.
target_compile_definitions(pythonEcsBinding PRIVATE _USE_MATH_DEFINES)


# Add include directories so the compiler can find flecs.h and other
# project headers.
target_include_directories(pythonEcsBinding PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../  # Access to all headers in src/
    target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_DEPS})
    ${flecs_INCLUDE_DIRS}
    ${flecs_INCLUDE_DIRS}/flecs # Add path for flecs.hpp
)

# Link against the flecs library, which is necessary for the reflection API.
target_link_libraries(pythonEcsBinding PRIVATE flecs::flecs_static)

# Tracy
find_package(Tracy REQUIRED)
target_link_libraries(pythonEcsBinding PRIVATE Tracy::TracyClient)

# HFSM2
find_package(hfsm2 REQUIRED CONFIG)
target_link_libraries(pythonEcsBinding PRIVATE hfsm2::hfsm2)

# Spdlog
find_package(spdlog REQUIRED)
target_link_libraries(pythonEcsBinding PRIVATE spdlog::spdlog)

# Eigen
find_package(Eigen3 REQUIRED CONFIG)
target_link_libraries(pythonEcsBinding PRIVATE Eigen3::Eigen)

# RapidCSV
find_package(rapidcsv REQUIRED CONFIG)
target_link_libraries(pythonEcsBinding PRIVATE rapidcsv::rapidcsv)





find_package(cglm REQUIRED CONFIG)
target_link_libraries(pythonEcsBinding PRIVATE cglm::cglm)
# The flecs libraries are written to #include "cglm.h", hence requiring this manual addition
target_include_directories(${PROJECT_NAME} PRIVATE ${cglm_INCLUDE_DIRS}/cglm)