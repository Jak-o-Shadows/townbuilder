cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

project(flecsOrbitSim LANGUAGES CXX)



set(FLECS_DIR build/)
find_package(FLECS REQUIRED)
find_package(Tracy REQUIRED)


add_executable(${PROJECT_NAME} src/main.cpp)
#target_sources(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp>")



# CMake downloads and sets up perturb for you
include(FetchContent)
FetchContent_Declare(
    perturb
    GIT_REPOSITORY "https://github.com/gunvirranu/perturb"
)
FetchContent_MakeAvailable(perturb)
target_link_libraries(${PROJECT_NAME} perturb)

target_link_libraries(${PROJECT_NAME} flecs::flecs_static)

# Using the global target name
target_link_libraries(${PROJECT_NAME} Tracy::TracyClient)