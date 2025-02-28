import os

from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy


class FlecsOrbitSimConan(ConanFile):
    name = "townBuilder"
    version = "0.0.1"
    license = "GPLv3"
    url = ""
    description = ""
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("flecs/4.0.0")
        self.requires("tracy/0.9.1")
        self.requires("spdlog/1.15.0")
        self.requires("eigen/3.4.0")

        # Game Things
        self.requires("recastnavigation/1.6.0")

        #ImGui Things
        self.requires("imgui/1.91.3")
        self.requires("glfw/3.3.8")
        self.requires("glew/2.2.0")

        # Leftover flecs GUI things
        self.requires("cglm/0.9.1")

    def generate(self):
        copy(self, "*glfw*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))
        copy(self, "*opengl3*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()