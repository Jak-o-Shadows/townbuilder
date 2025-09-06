import os

from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy
from conan.tools.cmake import CMakeToolchain
from conan.tools.cmake import CMakeDeps


class FlecsOrbitSimConan(ConanFile):
    name = "townBuilder"
    version = "0.0.1"
    license = "GPLv3"
    url = ""
    description = ""
    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.build_requires("cmake/4.1.1")

    def requirements(self):
        self.requires("flecs/4.0.4")
        self.requires("tracy/0.9.1")
        self.requires("spdlog/1.15.0", options={"use_std_fmt": True})
        self.requires("eigen/3.4.0")
        self.requires("hfsm2/2.5.2")
        self.requires("rapidcsv/8.84")
        self.requires("pybind11/3.0.1")  # Python bindings & embedded python

        # Game Things
        self.requires("recastnavigation/1.6.0")

        #ImGui Things
        self.requires("imgui/1.91.3")
        self.requires("glfw/3.3.8")
        self.requires("glew/2.2.0")

        # Leftover flecs GUI things
        self.requires("cglm/0.9.1")

        # For Swig bindings
        self.requires("swig/4.3.0")


    def generate(self):
        copy(self, "*glfw*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))
        copy(self, "*opengl3*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))
        #self.env_info.CMAKE_EXPORT_COMPILE_COMMANDS = "ON"  # For SonarQube
        
        toolchain = CMakeToolchain(self)
        toolchain.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True  # For SonarQube
        toolchain.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure({"CMAKE_EXPORT_COMPILE_COMMANDS": "ON"})  # For SonarQube
        cmake.build()