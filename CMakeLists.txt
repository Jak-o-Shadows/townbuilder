cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

project(townbuilder LANGUAGES C CXX)


set(DIR_DEPS ${CMAKE_CURRENT_LIST_DIR}/deps)
add_executable(${PROJECT_NAME} src/main.cpp)
message("deps " ${DIR_DEPS})
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logicPawn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/componentsPawn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ticks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderBasics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderNavmesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/componentsBuilding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/msgLogging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/coordinates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pathfinding.cpp
    ${DIR_DEPS}/flecs_components_cglm.c
    ${DIR_DEPS}/flecs_components_geometry.c
    ${DIR_DEPS}/flecs_components_graphics.c
    ${DIR_DEPS}/flecs_components_gui.c
#    ${DIR_DEPS}/flecs_components_input.c
    ${DIR_DEPS}/flecs_components_physics.c
    ${DIR_DEPS}/flecs_components_transform.c
#    ${DIR_DEPS}/flecs_systems_physics.c
#    ${DIR_DEPS}/flecs_systems_transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/Angle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/CoordinateConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/CoordinateSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simdis/Math.cpp
)# "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp>")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_DEPS})



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

# Spdlog
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog)

# Eigen
find_package(Eigen3 REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)

# RapidCSV
find_package(rapidcsv REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} rapidcsv::rapidcsv)


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