#pragma once

#include <flecs.h>

#include <GL/glew.h> // Initialize with glewInit()
#include "imgui.h"


// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>


namespace Render{

void part3(GLFWwindow *window);

struct Window{
    GLFWwindow* window;
    bool alive = true;
};


struct Box{
    float width;
    float height;
    float depth;
};


struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};

}