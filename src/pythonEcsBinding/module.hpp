#pragma once
 
#include <flecs.h>
#include <pybind11/embed.h>
#include <pybind11/subinterpreter.h>

#include <string>
 
namespace Python {


struct PythonFile {
    std::string filepath;
    size_t interpreter_idx;  // Couldn't get any kind of direct or pointer working, so bugger it
};

struct PythonFileUninitialised {};

struct components {
     components(flecs::world& ecs);
 };

struct systems {
     systems(flecs::world& ecs);
 };
 
 }
