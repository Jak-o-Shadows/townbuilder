cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

project(townbuilder LANGUAGES C CXX)


set(DIR_DEPS ${CMAKE_CURRENT_LIST_DIR}/deps)
add_executable(${PROJECT_NAME} src/main.cpp)
message("deps " ${DIR_DEPS})
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logicPawn.cpp
    ${DIR_DEPS}/flecs_components_cglm.c
    ${DIR_DEPS}/flecs_components_geometry.c
    ${DIR_DEPS}/flecs_components_graphics.c
    ${DIR_DEPS}/flecs_components_gui.c
    ${DIR_DEPS}/flecs_components_input.c
    ${DIR_DEPS}/flecs_components_physics.c
    ${DIR_DEPS}/flecs_components_transform.c
    ${DIR_DEPS}/flecs_game.c
    ${DIR_DEPS}/flecs_systems_physics.c
    ${DIR_DEPS}/flecs_systems_sokol.c
    ${DIR_DEPS}/flecs_systems_transform.c
)# "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp>")
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_DEPS})



# HFSM2
set(HFSM2_DIR tpl/hfsm2)
target_include_directories(${PROJECT_NAME} PRIVATE ${HFSM2_DIR}/include)


# Flecs
set(FLECS_DIR build)
find_package(flecs REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)


# Tracy
find_package(Tracy REQUIRED)
target_link_libraries(${PROJECT_NAME} Tracy::TracyClient)


find_package(cglm REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} cglm::cglm)
# The flecs libraries are written to #include "cglm.h", hence requiring this manual addition
target_include_directories(${PROJECT_NAME} PRIVATE ${cglm_INCLUDE_DIRS}/cglm)