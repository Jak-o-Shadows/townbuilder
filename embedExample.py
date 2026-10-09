import sys
import os
import pprint

try:  # Giant try-except to catch errors when running from C++
    print("embedExample")
    print(f"{__file__} working directory: {os.getcwd()}")
    # Add the build directory to the Python path
    rel_to_root = "build/Release/src/pythonEcsBinding"
    dir_base = os.path.dirname(os.path.abspath(__file__))
    dir_import = os.path.join(dir_base, rel_to_root)
    sys.path.append(dir_import)
    try:
        import pythonEcsBinding
    except ImportError as e:
        print(f"Failed to import pythonEcsBinding: {e}")
        raise e
    print("Imported pythonEcsBinding module")


    import sys
    import os

    def is_running_from_cpp():
        """
        Checks if the script is running from an embedded C++ interpreter.

        This works by inspecting the name of the running executable. If the script
        is run from the command line, the executable will be 'python.exe' or 'python'.
        If it's run from the C++ application, it will be the name of that compiled
        executable (e.g., 'townbuilder.exe').
        """
        executable_name = os.path.basename(sys.executable).lower()
        return not executable_name.startswith("python")

    def my_test_func():
        print("Hello from Python test function!")
        

    def init_from_cpp(world):
        print("Initialising from C++")
        pythonEcsBinding.init_ecs_bindings(world)
        print("Initialized ECS bindings from C++")

    if __name__ == "__main__":
        if not is_running_from_cpp():
            print("Running from Python interpreter")
            world = pythonEcsBinding.World()
            print(f"Created world object: {world}")
            pythonEcsBinding.init_ecs_bindings(world)
            print("Initialized ECS bindings from Python")

    pprint.pprint(dir(pythonEcsBinding))

except Exception as e:
    print(f"An error occurred: {e}")
    raise e