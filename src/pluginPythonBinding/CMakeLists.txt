cmake_minimum_required(VERSION 3.26.4)

project(pluginPythonBinding LANGUAGES CXX)


find_package(pybind11 REQUIRED CONFIG)
pybind11_add_module(pluginPythonBinding MODULE binding.cpp)
target_include_directories(pluginPythonBinding PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../  # To let it get normal townbuilder things
)

set_property(TARGET pluginPythonBinding PROPERTY CXX_STANDARD 20)

# Copy the test script in
add_custom_command(TARGET pluginPythonBinding
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test_python.py
        $<TARGET_FILE_DIR:pluginPythonBinding>
    COMMENT "Copying example python script to build directory for python binding"
)



