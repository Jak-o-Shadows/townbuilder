import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain


class FlecsOrbitSimConan(ConanFile):
    name = "flecsOrbitSim"
    version = "0.0.1"
    license = "GPLv3"
    url = ""
    description = ""
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("flecs/3.2.8")
        self.requires("tracy/0.9.1")

    def layout(self):
        self.folders.source = ""
        self.folders.build = "build"
        self.folders.generators = os.path.join(self.folders.build, "conan")
        self.folders.imports = os.path.join(self.folders.generators, "imports")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()