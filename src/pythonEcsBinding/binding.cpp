#include <flecs.h>
#include <flecs/addons/meta.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <tuple>
#include <iostream>

#include "msgLogging/module.hpp"

#include "component_registry.h"

// Forward declaration for our main world object
flecs::world* g_ecs = nullptr;

namespace py = pybind11;

/**
* @brief A template function to bind a single component struct to Python.
*
* This function uses flecs reflection to iterate over the members of a struct,
* exposing each as a read-write property in Python. It dynamically handles
* primitive types like float, int, and std::string.
*
* @tparam T The component type to bind.
* @param m The pybind11 module to add the binding to.
*/

template <typename T>
void bind_component(const py::module_& m) {
    // The world must be available, and components registered, to access reflection data.
    if (!g_ecs) {
        throw std::runtime_error("flecs::world not initialized for pybind module.");
    }
    std::cout << g_ecs << std::endl;

    // Get the expected component name from the C++ type
    std::string expected_name = std::string(typeid(T).name());
    // Clean up the mangled type name: remove "struct " and "class " prefixes
    if (expected_name.find("struct ") == 0) {
        expected_name = expected_name.substr(7);
    } else if (expected_name.find("class ") == 0) {
        expected_name = expected_name.substr(6);
    }
    // Keep C++ scope notation (::). We'll lookup the component entity using
    // a leading C++-style path (e.g. ::Plugin::components::PluginGuid).
    std::cout << "  Expected component name (from typeid): " << expected_name << std::endl;
    std::cout << "  sizeof(T): " << sizeof(T) << std::endl;
    
    // Lookup using C++-style scope separators (::). Try exact path with a
    // leading ::. If not found, try inserting ::components:: after the
    // first namespace (e.g. ::Plugin::components::PluginGuid).
    std::string lookup_name = expected_name;
    if (lookup_name.rfind("::", 0) != 0) {
        // prefix with leading :: if missing
        lookup_name = std::string("::") + lookup_name;
    }
    flecs::entity comp = g_ecs->lookup(lookup_name.c_str());
    std::string found_name;
    if (comp) {
        found_name = lookup_name;
    } else {
        // try inserting ::components:: before the last namespace segment
        size_t last_sep = expected_name.rfind("::");
        if (last_sep != std::string::npos) {
            std::string prefix = expected_name.substr(0, last_sep);
            std::string suffix = expected_name.substr(last_sep + 2);
            std::string with_components = std::string("::") + prefix + "::components::" + suffix;
            comp = g_ecs->lookup(with_components.c_str());
            if (comp) found_name = with_components;
        }
    }

    if (!comp) {
        std::cerr << "ERROR: Could not find component entity for: " << expected_name << " or with ::components:: inserted" << std::endl;
        throw std::runtime_error("Component entity not found: " + expected_name);
    }

    std::cout << "  Found component entity id: " << comp.id() << std::endl;
    std::cout << "  Component entity name: " << comp.name() << " (matched: " << found_name << ")" << std::endl;

    // Create a Python class for the component using the entity's short name
    // (e.g. PluginGuid) to keep Python class names tidy.
    py::class_<T> component_class(m, comp.name());
    component_class.def(py::init<>());

    if (!comp.has<EcsType>()) {
        std::cerr << "Component '" << comp.name() << "' has no EcsType metadata - skipping binding." << std::endl;
        return;
    }
    const EcsType& type = comp.get<EcsType>();
    
    try {
        std::cout << "Binding component for entity: " << found_name << std::endl;
        if (type.kind == ecs_type_kind_t::EcsStructType){
            std::cout << "Binding struct component: " << found_name << std::endl;
            const EcsStruct& struct_info = comp.get<EcsStruct>();
            if (struct_info.members.count > 0) {
                std::cout << "  Members found: " << struct_info.members.count << std::endl;
                std::cout << "  About to iterate members" << std::endl;
                for (size_t memberIdx=0; memberIdx < struct_info.members.count; memberIdx++) {
                    std::cout << "    memberIdx: " << memberIdx << std::endl;
                    const ecs_member_t* member = (const ecs_member_t*)ecs_vec_get_t(&struct_info.members, ecs_member_t, memberIdx);
                    if (!member) {
                        std::cerr << "    ecs_vec_get_t returned null for memberIdx " << memberIdx << std::endl;
                        continue;
                    }
                    const char* member_name = member->name;
                    ecs_entity_t member_type_ent = member->type;
                    int member_offset = member->offset;
                    
                    // Add bindings based on the member's C++ type.
                    if (member_type_ent == g_ecs->component<float>()) {
                        component_class.def_property(member_name,
                            [member_offset](const T &c) {
                                return *reinterpret_cast<const float*>(reinterpret_cast<const char*>(&c) + member_offset);
                            },
                            [member_offset](T &c, float value) {
                                *reinterpret_cast<float*>(reinterpret_cast<char*>(&c) + member_offset) = value;
                            });
                    } else if (member_type_ent == g_ecs->component<int>()) {
                        component_class.def_property(member_name,
                            [member_offset](const T &c) {
                                return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(&c) + member_offset);
                            },
                            [member_offset](T &c, int value) {
                                *reinterpret_cast<int*>(reinterpret_cast<char*>(&c) + member_offset) = value;
                            });
                    } else if (member_type_ent == g_ecs->component<std::string>()) {
                        component_class.def_property(member_name,
                            [member_offset](const T &c) -> const std::string& { return *reinterpret_cast<const std::string*>(reinterpret_cast<const char*>(&c) + member_offset); },
                            [member_offset](T &c, const std::string &value) { *reinterpret_cast<std::string*>(reinterpret_cast<char*>(&c) + member_offset) = value;
                            });
                    } else if (member_type_ent == g_ecs->component<uint64_t>()) {
                        component_class.def_property(member_name,
                            [member_offset](const T &c) {
                                return *reinterpret_cast<const uint64_t*>(reinterpret_cast<const char*>(&c) + member_offset);
                            },
                            [member_offset](T &c, uint64_t value) {
                                *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(&c) + member_offset) = value;
                            });
                    } else {
                        std::cout << "  Skipping unsupported member type for member: " << member_name << std::endl;
                    }
                }
            } else {
                std::cout << "  No members found in struct component: " << expected_name << std::endl;
            }
                            
        } else{
            std::cout << "Skipping non-struct component: " << expected_name << std::endl;
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error while binding component: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Error while binding component: " << e.what() << std::endl;
        throw;
    }
}
                
/**
* @brief A helper function to iterate over a tuple of types and apply a function.
*/
template <typename Tuple, typename Func, std::size_t... Is>
void for_each_in_tuple(Func &&f, std::index_sequence<Is...>) {
    // The fold expression unpacks the tuple types and calls the function for each.
    (f.template operator()<std::tuple_element_t<Is, Tuple>>(), ...);
}

void init_ecs_bindings(pybind11::object world_obj) {
    g_ecs = world_obj.cast<flecs::world*>();
    if (!g_ecs) {
        throw std::runtime_error("Failed to cast Python world object to flecs::world*");
    }
    std::cout <<g_ecs << std::endl;
    
    // Import the extension module inside the current interpreter/subinterpreter
    // so it is initialized for this interpreter's state.
    py::module_ m;
    try {
        m = py::module_::import("pythonEcsBinding");
        std::cout << "Python binding Module Imported" << std::endl;
    } catch (const py::error_already_set &e) {
        std::cerr << "Failed to import pythonEcsBinding: " << e.what() << std::endl;
        // Ensure any pending Python error is printed to stderr for visibility.
        PyErr_Print();
        throw std::runtime_error(std::string("Failed to import pythonEcsBinding: ") + e.what());
    }
    

    
    // Create a lambda that calls bind_component for a given type
    // Capture m by value to ensure it stays valid throughout the binding process
    auto bind_all = [m]<typename T>() mutable { bind_component<T>(m); };
    std::cout << "Created binding lambda" << std::endl;
    
    try {
        // Iterate over all components in our AllComponents tuple and bind them
        for_each_in_tuple<AllComponents>(bind_all, std::make_index_sequence<std::tuple_size_v<AllComponents>>{});
        std::cout << "All components bound" << std::endl;
    } catch (const py::error_already_set &e) {
        std::cerr << "Python error binding components: " << e.what() << std::endl;
        PyErr_Print();
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Error binding components: " << e.what() << std::endl;
        // If Python left an error, make sure it is printed for debugging.
        PyErr_Print();
        throw;
    }
}

void init_from_python(flecs::world& ecs) {
    g_ecs = &ecs;

    // Logger imported first as the other modules use it on their import
    g_ecs->import<Logging::components>();
    g_ecs->import<Logging::systems>();
    std::cout << "Logger imported" << std::endl;
    // Then import all the components
    g_ecs->import<Buildings::components>();
    g_ecs->import<Coordinates::components>();
    //g_ecs->import<fdis::components>();
    //g_ecs->import<Map::components>();
    //g_ecs->import<Pathfinding::components>();
    g_ecs->import<Pawn::components>();
    g_ecs->import<Plugin::components>();
    //g_ecs->import<Python::components>();
    //g_ecs->import<Render::components>();
    g_ecs->import<Statemachine::components>();
    g_ecs->import<UI::components>();
    std::cout << "Components imported" << std::endl;

    init_ecs_bindings(py::cast(g_ecs));
}



PYBIND11_MODULE(pythonEcsBinding, m) {
    m.doc() = "Automated bindings for TownBuilder ECS components";
    
    // Expose the flecs::world class to Python
    py::class_<flecs::world>(m, "World")
    .def(py::init<>()); // Allow Python to create new flecs::world instances
    
    m.def("init_ecs_bindings", &init_ecs_bindings, "Initialize ECS bindings with existing Flecs world",
        py::arg("world_obj"));

    m.def("init_from_python", &init_from_python, "Initialize ECS from Python-created Flecs world",
        py::arg("ecs"));
}
                    