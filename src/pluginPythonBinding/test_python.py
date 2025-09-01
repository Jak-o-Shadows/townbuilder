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
    tick_input.time = 3.33

    # Set the channels
    channels = []
    for channel_idx in range(4):
        channel = pluginPythonBinding.ComplexChannel()
        #channel = tick_input.channels[channel_idx]

        # Create a numpy array and assign it. This allocates memory
        # on the Python side and makes the C++ side point to it.
        numpy_data = np.array([channel_idx/2+1j, channel_idx+0j, 0 + channel_idx*1j], dtype=np.complex128)
        print(numpy_data)
        print(hex(numpy_data.ctypes.data))
        channel.data = numpy_data

        # The 'count' member is automatically updated
        print(f"Channel {channel_idx} count: {channel.count} maxloc: {np.argmax(np.abs(numpy_data))}")
        channels.append(channel)
    tick_input.channels = channels

    # Create and populate a ComplexMapArray and assign it to the TickInput.
    # This assumes 'TickInput' has a member 'complex_map' of type 'ComplexMapArray'.
    complex_map = pluginPythonBinding.ComplexMapArray()
    rows, cols = 5, 10
    # Create a list of 4 random numpy arrays
    random_arrays = [
        (np.random.rand(rows, cols) + 1j * np.random.rand(rows, cols)).astype(np.complex128)
        for _ in range(4)
    ]
    for arr in random_arrays:
        print(hex(arr.ctypes.data))
    print("of the list of arrays", hex(id(random_arrays)))
    # Assign the numpy arrays to the data property.
    # The C++ object will now point to the memory of these numpy arrays.
    complex_map.data = random_arrays
    print(f"\nComplex map rows: {complex_map.rows}, cols: {complex_map.cols}")
    tick_input.complex_map = complex_map


    # Test out the plugin interface

    p = pluginPythonBinding.PluginLoader()
    p.plugin_init()
    guid = 20250831
    p.entity_init(guid)
    p.plugin_tick(guid, tick_input)
    results = p.plugin_get_results(guid)
    print(f"Results: {results.success}")
    print(f"{results.max_location1}, {results.max_location2}, {results.max_location3}, {results.max_location4}")
    print(f"{results.first_elem}, ({results.num_rows}, {results.num_cols})")
