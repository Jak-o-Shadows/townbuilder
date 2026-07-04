cmake_minimum_required(VERSION 3.26.4)
cmake_policy(SET CMP0063 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(taskflow_demonstrator LANGUAGES C CXX)
# SonarQube: Generate compilation database for analysis
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} main.cpp)
set(DIR_TOWNBUILDER ${CMAKE_CURRENT_LIST_DIR}/../../../)
target_sources(${PROJECT_NAME} PRIVATE
    ${DIR_TOWNBUILDER}/src/taskflow_scheduler/components.cpp
    ${DIR_TOWNBUILDER}/src/taskflow_scheduler/systems.cpp
    # Grabbing the townbuiler msgLogging because we'll need em
    ${DIR_TOWNBUILDER}/src/msgLogging/components.cpp
    ${DIR_TOWNBUILDER}/src/msgLogging/systems.cpp
    ${DIR_TOWNBUILDER}/src/msgLogging/db_sink.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_TOWNBUILDER}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_DEPS})

# Define NOMINMAX for Windows builds to prevent conflicts with <windows.h>
# This is a common issue with libraries like Tracy that use std::numeric_limits::max.
# This is straight from Gemini, so who knows if it's right, but annoyingly it worked
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif()

# Flecs
set(FLECS_DIR build)
find_package(flecs REQUIRED)
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)

# Tracy
find_package(Tracy REQUIRED)
target_link_libraries(${PROJECT_NAME} Tracy::TracyClient)
target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE FLECS_PERF_TRACE)

# Spdlog & SOCI for logging (SOCI is a database sink for Spdlog)
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog)
find_package(soci REQUIRED)
target_link_libraries(${PROJECT_NAME} soci::soci)

# Copy compile_commands.json to a consistent location after build
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to source directory"
)