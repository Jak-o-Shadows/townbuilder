cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

project(flecsOrbitSim LANGUAGES CXX)







# Flecs-Hub Modules
set(FLECS_HUB_BASE ${CMAKE_CURRENT_SOURCE_DIR}/src/flecs-hub)
set(FLECS_HUB_INCS
#    ${FLECS_HUB_BASE}/flecs-components-graphics
#    ${FLECS_HUB_BASE}/flecs-components-gui/include
    ${FLECS_HUB_BASE}/flecs-components-cglm/include
    ${FLECS_HUB_BASE}/flecs-components-transform/include
#    ${FLECS_HUB_BASE}/flecs-game/include
#    ${FLECS_HUB_BASE}/flecs-systems-physics/include
#    ${FLECS_HUB_BASE}/flecs-systems-sokol/include
#    ${FLECS_HUB_BASE}/flecs-systems-transform/include
)
set(FLECS_HUB_SRCS
#    ${FLECS_HUB_BASE}/flecs-components-graphics/src/graphics.c
#    ${FLECS_HUB_BASE}/flecs-components-graphics/flecs_components_graphics.c
    
    #${FLECS_HUB_BASE}/deps/flecs_components_transform.c
    ${FLECS_HUB_BASE}/flecs-components-transform/src/transform.c

    #${FLECS_HUB_BASE}/flecs-systems-transform/src/main.c
    #${FLECS_HUB_BASE}/deps/flecs_systems_transform.c

)




add_executable(${PROJECT_NAME} src/main.cpp)
#target_sources(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp>")
target_sources(${PROJECT_NAME} PRIVATE ${FLECS_HUB_SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${FLECS_HUB_INCS})


# CMake downloads and sets up perturb for you
include(FetchContent)
FetchContent_Declare(
    perturb
    GIT_REPOSITORY "https://github.com/gunvirranu/perturb"
)
FetchContent_MakeAvailable(perturb)
target_link_libraries(${PROJECT_NAME} perturb)

# Flecs
set(FLECS_DIR build)
find_package(FLECS REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)

# Tracy
find_package(Tracy REQUIRED)
target_link_libraries(${PROJECT_NAME} Tracy::TracyClient)


find_package(cglm REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} cglm::cglm)
