from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.cmake import CMakeToolchain
from conan.tools.cmake import CMakeDeps


class TaskflowDemonstrator(ConanFile):
    name = "Taskflow_Demonstrator"
    version = "0.0.1"
    license = "GPLv3"
    url = ""
    description = ""
    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.build_requires("cmake/4.1.1")
        self.build_requires("ninja/1.11.1")

    def requirements(self):
        self.requires("flecs/4.1.1")
        self.requires("tracy/0.9.1")
        self.requires("taskflow/4.0.0")

        # Required for logging integration with townbuilder
        self.requires("spdlog/1.15.0", options={"use_std_fmt": True})
        self.requires("soci/4.1.2", options={"with_sqlite3": True})


    def generate(self):
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