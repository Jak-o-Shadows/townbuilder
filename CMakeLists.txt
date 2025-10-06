cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(townbuilder LANGUAGES C CXX)
# SonarQube: Generate compilation database for analysis
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DIR_DEPS ${CMAKE_CURRENT_LIST_DIR}/deps)
add_executable(${PROJECT_NAME} src/main.cpp)
message("deps " ${DIR_DEPS})
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/buildings/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/coordinates/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/coordinates/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dis/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dis/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/map/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/msgLogging/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/msgLogging/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pathfinding/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pawn/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pawn/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pawn/statemachine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pythonEcsBinding/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pythonEcsBinding/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/renderNavmesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/renderBasics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/statemachine/components.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/statemachine/systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ticks/module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/components.cpp
    ${DIR_DEPS}/flecs_components_cglm.c
    ${DIR_DEPS}/flecs_components_geometry.c
    ${DIR_DEPS}/flecs_components_graphics.c
    ${DIR_DEPS}/flecs_components_gui.c
    ${DIR_DEPS}/flecs_components_physics.c
    ${DIR_DEPS}/flecs_components_transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/Angle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/CoordinateConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/CoordinateSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/Math.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_DEPS})

# Define NOMINMAX for Windows builds to prevent conflicts with <windows.h>
# This is a common issue with libraries like Tracy that use std::numeric_limits::max.
# This is straight from Gemini, so who knows if it's right, but annoyingly it worked
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif()


# HFSM2
find_package(hfsm2 REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} hfsm2::hfsm2)


# Flecs
set(FLECS_DIR build)
find_package(flecs REQUIRED)
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)


# Tracy
find_package(Tracy REQUIRED)
target_link_libraries(${PROJECT_NAME} Tracy::TracyClient)
target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE FLECS_PERF_TRACE)

# Spdlog
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog)

# Eigen
find_package(Eigen3 REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)

# RapidCSV
find_package(rapidcsv REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} rapidcsv::rapidcsv)

# SOCI
find_package(soci REQUIRED)
target_link_libraries(${PROJECT_NAME} soci::soci)

#RecastNavigation
find_package(recastnavigation REQUIRED)
target_link_libraries(${PROJECT_NAME} recastnavigation::recastnavigation)
target_include_directories(${PROJECT_NAME} PRIVATE ${recastnavigation_INCLUDE_DIRS})

# KDIS
set(DIS_VERSION 7 CACHE STRING "setting dis version should be done")
target_compile_definitions(${PROJECT_NAME} PRIVATE DIS_VERSION=${DIS_VERSION})
target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/tpl/kdis/build/Debug/KDISd.lib)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tpl/kdis/build/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tpl/kdis/include)


# Dear IMGUI
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
#add_custom_command(TARGET ${PROJECT_NAME}
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.vs $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.fs $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)
target_sources(${PROJECT_NAME} PRIVATE
    bindings/imgui_impl_glfw.cpp
    bindings/imgui_impl_opengl3.cpp
)
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(${PROJECT_NAME} imgui::imgui GLEW::GLEW glfw)





find_package(cglm REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} cglm::cglm)
# The flecs libraries are written to #include "cglm.h", hence requiring this manual addition
target_include_directories(${PROJECT_NAME} PRIVATE ${cglm_INCLUDE_DIRS}/cglm)

# PyBind11 is used for both the python embedded interpreter, and the
#   Python bindings to the plugin system
find_package(pybind11 REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} pybind11::embed)



# Example Plugin
add_subdirectory(src/pluginExample)
add_dependencies(${PROJECT_NAME} plugin)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:plugin>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying pluginExample to build directory"
)

# Python Plugin Binding
#   Be able to test the plugin via Python
add_subdirectory(src/pluginPythonBinding)
add_dependencies(pluginPythonBinding plugin)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:plugin>
        $<TARGET_FILE_DIR:pluginPythonBinding>
    COMMENT "Copying pluginExample to build directory for python binding"
)

# Python embedding - Pybind module for writing Python scripts
add_subdirectory(src/pythonEcsBinding)
