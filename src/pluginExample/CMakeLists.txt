cmake_minimum_required(VERSION 3.26)
project(plugin LANGUAGES CXX)

add_library(plugin SHARED
    impl.cpp
)

target_include_directories(plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin
)

find_package(flecs REQUIRED)
target_link_libraries(plugin PRIVATE flecs::flecs_static)

# If your plugin interface has other dependencies, add them here