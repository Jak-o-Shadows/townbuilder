cmake_minimum_required(VERSION 3.26)
project(plugin LANGUAGES CXX)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)  # TODO: Remove this and use __declspec(dllexport) instead, but that wasn't working, so who cares

add_library(plugin SHARED
    impl.cpp
    ../plugin/interface.hpp
)

target_include_directories(plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin
)


find_package(flecs REQUIRED)
target_link_libraries(plugin PRIVATE flecs::flecs_static)

# Eigen
find_package(Eigen3 REQUIRED CONFIG)
target_link_libraries(plugin PRIVATE Eigen3::Eigen)


target_compile_definitions(plugin PRIVATE PLUGIN_BUILD_DLL)