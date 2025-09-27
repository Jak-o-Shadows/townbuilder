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
flecs::world* g_ecs;

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
void bind_component(py::module_ &m) {
    // The world must be available, and components registered, to access reflection data.
    if (!g_ecs) {
        throw std::runtime_error("flecs::world not initialized for pybind module.");
    }

    auto comp = g_ecs->component<T>();
    std::string component_name = std::string(comp.name());

    // Create a Python class for the component
    py::class_<T> component_class(m, component_name.c_str());
    component_class.def(py::init<>());


    const EcsType& type = comp.get<EcsType>();
    if (type.kind == ecs_type_kind_t::EcsStructType){
        std::cout << "Binding struct component: " << component_name << std::endl;
        const EcsStruct& struct_info = comp.get<EcsStruct>();
        if (struct_info.members.count > 0) {
            std::cout << "  Members found: " << struct_info.members.count << std::endl;
            for (size_t memberIdx=0; memberIdx < struct_info.members.count; memberIdx++) {
            const ecs_member_t* member = (const ecs_member_t*)ecs_vec_get_t(&struct_info.members, ecs_member_t, memberIdx);
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
                } else {
                    std::cout << "  Skipping unsupported member type for: " << member_name << std::endl;
                }
            }
        } else {
            std::cout << "  No members found in struct component: " << component_name << std::endl;
        }

    } else{
        std::cout << "Skipping non-struct component: " << component_name << std::endl;
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


PYBIND11_MODULE(pythonEcsBinding, m) {
    m.doc() = "Automated bindings for TownBuilder ECS components";

    flecs::world ecs;
    // Logger imported first as the other modules use it on their import
    ecs.import<Logging::components>();
    ecs.import<Logging::systems>();
    std::cout << "Logger imported" << std::endl;
    // Then import all the components
    ecs.import<Buildings::components>();
    ecs.import<Coordinates::components>();
    //ecs.import<fdis::components>();
    //ecs.import<Map::components>();
    //ecs.import<Pathfinding::components>();
    ecs.import<Pawn::components>();
    ecs.import<Plugin::components>();
    //ecs.import<Python::components>();
    //ecs.import<Render::components>();
    //ecs.import<Statemachine::components>();
    ecs.import<UI::components>();

    g_ecs = &ecs;

    // Create a lambda that calls bind_component for a given type
    auto bind_all = [&m]<typename T>() { bind_component<T>(m); };

    // Iterate over all components in our AllComponents tuple and bind them
    for_each_in_tuple<AllComponents>(bind_all, std::make_index_sequence<std::tuple_size_v<AllComponents>>{});
}
