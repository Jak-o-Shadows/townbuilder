import sys
import pprint

try:  # Giant try-except to catch errors when running from C++
    print("embedExample")
    # Add the build directory to the Python path
    sys.path.append("build/src/pythonEcsBinding/Release")  # For running from command line
    sys.path.append("../src/pythonEcsBinding/Release")  # For running from C++
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