#pragma once
 
#include <flecs.h>
#include <pybind11/embed.h>
#include <pybind11/subinterpreter.h>

#include <string>
 
namespace Python {

//extern std::vector<pybind11::subinterpreter> interpreters;

struct PythonFile {
    std::string filepath;
   size_t interpreter_idx;  // Couldn't get any kind of direct or pointer working, so bugger it
};

struct components {
     components(flecs::world& ecs);
 };

struct systems {
     systems(flecs::world& ecs);
 };
 
 }
