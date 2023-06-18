cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0063 NEW)

project(townbuilder LANGUAGES CXX)



set(FLECS_DIR build/)
find_package(FLECS REQUIRED)


add_executable(${PROJECT_NAME} src/main.cpp)


target_link_libraries(${PROJECT_NAME} flecs::flecs_static)
