#pragma once

#include <flecs.h>

#include <GL/glew.h> // Initialize with glewInit()

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>


namespace Render{

void part3(GLFWwindow *window);

struct Window{
    GLFWwindow* window;
};

struct module {
    module(flecs::world& ecs);
   
};

}