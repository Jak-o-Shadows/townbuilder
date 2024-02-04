import os

from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class FlecsOrbitSimConan(ConanFile):
    name = "townBuilder"
    version = "0.0.1"
    license = "GPLv3"
    url = ""
    description = ""
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("flecs/3.2.8")
        self.requires("tracy/0.9.1")
        self.requires("cglm/0.9.1")
    
    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()