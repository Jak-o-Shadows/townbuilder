cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

project(townbuilder LANGUAGES CXX)



set(FLECS_DIR build/)
find_package(FLECS REQUIRED)

find_package(Tracy REQUIRED)


add_executable(${PROJECT_NAME} src/main.cpp)
target_sources(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/src/gridMap.cpp>")



# HFSM2
set(HFSM2_DIR tpl/hfsm2)
target_include_directories(${PROJECT_NAME} PRIVATE ${HFSM2_DIR}/include)



target_link_libraries(${PROJECT_NAME} flecs::flecs_static)

# Using the global target name
target_link_libraries(${PROJECT_NAME} Tracy::TracyClient)