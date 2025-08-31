import numpy as np

import pluginPythonBinding

if __name__ == "__main__":

    print(dir(pluginPythonBinding))

    print(pluginPythonBinding.add(3, 4))
    print(pluginPythonBinding.hello())


    # Create a channel
    channel = pluginPythonBinding.ComplexChannel()
    numpy_data = np.array([10+2j, 30+4j, 50+6j], dtype=np.complex128)
    channel.data = numpy_data
    print(f"Channel count: {channel.count}")


    # Create a TickInput object
    # When tick_input (and thus channel_0) goes out of scope,
    # the numpy array will be garbage collected by Python.
    tick_input = pluginPythonBinding.TickInput()

    # Get a reference to one of its channels
    for channel_idx in range(4):
        channel = tick_input.channels[channel_idx]

        # Create a numpy array and assign it. This allocates memory
        # on the Python side and makes the C++ side point to it.
        numpy_data = np.array([channel_idx+1+2j, 3033+4j, 5+6j], dtype=np.complex128)
        channel.data = numpy_data

        # The 'count' member is automatically updated
        print(f"Channel {channel_idx} count: {channel.count}")

    # Test out the plugin interface

    p = pluginPythonBinding.PluginLoader()
    p.plugin_init()
    guid = 20250831
    p.entity_init(guid)
    p.plugin_tick(guid, tick_input)
    results = p.plugin_get_results(guid)
    print(f"Results: {results.success}, {results.max_location}")
