#include "plugin/interface.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/native_enum.h>
#include <complex>
#include <string>
#include <stdexcept>
#include <iostream>
#include <format>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static int add(int i, int j) {
    return i + j;
}

static const char *hello() {
    return "Hello, World!";
}


struct PluginLoader {
    #ifdef _WIN32
    HMODULE pluginLibrary = nullptr;
    #else
    void* pluginLibrary = nullptr;
    #endif

    typedef void (*PluginInitFunc)();
    typedef void (*PluginTickFunc)(Plugin::GUID, struct Plugin::TickInput);
    typedef Plugin::PluginResults (*PluginGetResultsFunc)(Plugin::GUID);
    typedef void (*EntityInitFunc)(Plugin::GUID);

    PluginInitFunc plugin_init_ptr = nullptr;
    PluginTickFunc plugin_tick_ptr = nullptr;
    PluginGetResultsFunc plugin_get_results_ptr = nullptr;
    EntityInitFunc entity_init_ptr = nullptr;

    PluginLoader() {
        // Load the DLL at runtime
        #ifdef _WIN32
        pluginLibrary = LoadLibraryA("plugin.dll");
        if (!pluginLibrary) {
            DWORD errorCode = GetLastError();
            LPSTR messageBuffer = NULL;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            std::string message(messageBuffer, size - 2); // -2 to remove \r\n
            LocalFree(messageBuffer);
            throw std::runtime_error(std::format("Failed to load plugin.dll. Code {}: {}", errorCode, message));
        }

        plugin_init_ptr = (PluginInitFunc)GetProcAddress(pluginLibrary, "plugin_init");
        entity_init_ptr = (EntityInitFunc)GetProcAddress(pluginLibrary, "entity_init");
        plugin_tick_ptr = (PluginTickFunc)GetProcAddress(pluginLibrary, "plugin_tick");
        plugin_get_results_ptr = (PluginGetResultsFunc)GetProcAddress(pluginLibrary, "plugin_get_results");

        #else
        pluginLibrary = dlopen("plugin.so", RTLD_LAZY);
        if (!pluginLibrary) {
            throw std::runtime_error(std::format("Failed to load plugin.so: {}", dlerror()));
        }

        plugin_init_ptr = (PluginInitFunc)dlsym(pluginLibrary, "plugin_init");
        plugin_tick_ptr = (PluginTickFunc)dlsym(pluginLibrary, "plugin_tick");
        plugin_get_results_ptr = (PluginGetResultsFunc)dlsym(pluginLibrary, "plugin_get_results");
        entity_init_ptr = (EntityInitFunc)dlsym(pluginLibrary, "entity_init");   
        #endif //   NOT _WIN32
        
        if (!plugin_init_ptr || !plugin_tick_ptr || !plugin_get_results_ptr || !entity_init_ptr) {
            // The destructor will be called automatically if the constructor throws,
            // and it will correctly free the library handle.
            throw std::runtime_error(std::format("Failed to get function address from plugin file: plugin_init {}, plugin_tick {}, plugin_get_results {}, entity_init {}",
                plugin_init_ptr != nullptr,
                plugin_tick_ptr != nullptr,
                plugin_get_results_ptr != nullptr,
                entity_init_ptr != nullptr));
        }
    }

    ~PluginLoader() {
        if (pluginLibrary) {
            #ifdef _WIN32
            FreeLibrary(pluginLibrary);
            #else
            dlclose(pluginLibrary);
            #endif
        }
    }

    void plugin_init() {
        plugin_init_ptr();
    }

    void plugin_tick(Plugin::GUID guid, Plugin::TickInput& tick_input) {
        plugin_tick_ptr(guid, tick_input);
    }

    void entity_init(Plugin::GUID guid) {
        entity_init_ptr(guid);
    }

    Plugin::PluginResults plugin_get_results(Plugin::GUID guid) {
        return plugin_get_results_ptr(guid);
    }
};

PYBIND11_MODULE(pluginPythonBinding, m) {
    m.doc() = "Python binding for TownBuilder plugin system"; // optional module docstring

    m.def("add", &add, "A function which adds two numbers");
    m.def("hello", &hello, "A function which returns 'Hello, World!'");

    pybind11::class_<Plugin::PluginResults>(m, "PluginResults", "Results from a plugin operation")
        .def(pybind11::init<>())
        .def_readwrite("success", &Plugin::PluginResults::success)
        .def_readwrite("max_location1", &Plugin::PluginResults::max_location1)
        .def_readwrite("max_location2", &Plugin::PluginResults::max_location2)
        .def_readwrite("max_location3", &Plugin::PluginResults::max_location3)
        .def_readwrite("max_location4", &Plugin::PluginResults::max_location4)
        .def_readwrite("first_elem", &Plugin::PluginResults::first_elem)
        .def_readwrite("num_rows", &Plugin::PluginResults::num_rows)
        .def_readwrite("num_cols", &Plugin::PluginResults::num_cols);


    pybind11::class_<Plugin::ComplexChannel>(m, "ComplexChannel", "A complex channel for plugin communication", pybind11::dynamic_attr())
        .def(pybind11::init<>())
        .def_readonly("count", &Plugin::ComplexChannel::count, "Number of complex values in the channel.")
        .def_property("data",
            /* Getter: returns the python-owned numpy array */
            [](pybind11::object &self) -> pybind11::array {
                if (pybind11::hasattr(self, "_data_owner")) {
                    return self.attr("_data_owner").cast<pybind11::array>();
                } else {
                    // If no data has been assigned from Python, return an empty array.
                    return pybind11::array();
                }
            },
            /* Setter: Python (numpy) allocates and owns the memory. */
            [](pybind11::object &self, pybind11::array_t<std::complex<double>, pybind11::array::c_style | pybind11::array::forcecast> arr) {
                Plugin::ComplexChannel &c = self.cast<Plugin::ComplexChannel&>();
                pybind11::buffer_info info = arr.request();
                if (info.ndim != 1) {
                    throw std::runtime_error("Incompatible buffer dimension! Expected a 1D array.");
                }
                if (info.format != pybind11::format_descriptor<std::complex<double>>::format()) {
                    throw std::runtime_error("Incompatible format: expected a numpy array of complex doubles.");
                }
                c.data = static_cast<std::complex<double>*>(info.ptr);
                c.count = info.shape[0];
                self.attr("_data_owner") = arr;
            }, "A numpy array view of the channel data. Can be assigned from a numpy array.");


    pybind11::class_<Plugin::ComplexMapArray>(m, "ComplexMapArray", "A 4-channel complex map for plugin communication", pybind11::dynamic_attr())
        .def(pybind11::init<>())
        .def_readonly("rows", &Plugin::ComplexMapArray::rows)
        .def_readonly("cols", &Plugin::ComplexMapArray::cols)
        .def_property("data",
            /* Getter: returns the python-owned numpy arrays */
            [](pybind11::object &self) -> pybind11::tuple {
                if (pybind11::hasattr(self, "_data_owner")) {
                    return self.attr("_data_owner").cast<pybind11::tuple>();
                } else {
                    // If no data has been assigned from Python, return an empty tuple of arrays.
                    return pybind11::make_tuple(pybind11::array(), pybind11::array(), pybind11::array(), pybind11::array());
                }
            },
            /* Setter: Python (numpy) allocates and owns the memory. */
            [](pybind11::object &self, const pybind11::sequence& seq) {
                if (seq.size() != 4) {
                    throw std::runtime_error("Input sequence must have 4 numpy arrays.");
                }

                Plugin::ComplexMapArray &cma = self.cast<Plugin::ComplexMapArray&>();

                // Use the first array to determine dimensions
                auto first_arr = seq[0].cast<pybind11::array_t<std::complex<double>, pybind11::array::c_style | pybind11::array::forcecast>>();
                pybind11::buffer_info first_info = first_arr.request();

                if (first_info.ndim != 2) {
                    throw std::runtime_error("Numpy arrays must be 2-dimensional.");
                }

                cma.rows = first_info.shape[0];
                cma.cols = first_info.shape[1];
                cma.data[0] = static_cast<std::complex<double>*>(first_info.ptr);

                // Verify other arrays and set pointers
                for (size_t i = 1; i < 4; ++i) {
                    auto arr = seq[i].cast<pybind11::array_t<std::complex<double>, pybind11::array::c_style | pybind11::array::forcecast>>();
                    pybind11::buffer_info info = arr.request();

                    if (info.ndim != 2 || (size_t)info.shape[0] != cma.rows || (size_t)info.shape[1] != cma.cols) {
                        throw std::runtime_error("All numpy arrays must have the same 2D shape.");
                    }
                    cma.data[i] = static_cast<std::complex<double>*>(info.ptr);
                }

                // Store the python objects to manage their lifetime
                self.attr("_data_owner") = seq;
            }, "A tuple of 4 numpy array views of the map data. Can be assigned from a list/tuple of 4 numpy arrays.");


    pybind11::native_enum<Plugin::Mode>(m, "Mode", "enum.Enum")
        .value("a", Plugin::Mode::a)
        .value("b", Plugin::Mode::b)
        .value("c", Plugin::Mode::c)
        .export_values()
        .finalize();

    
    pybind11::class_<Plugin::TickInput>(m, "TickInput", "Input for a plugin tick, contains time and channel data", pybind11::dynamic_attr())
        .def(pybind11::init<>())
        .def_readwrite("mode", &Plugin::TickInput::mode)
        .def_property("channels",
            /* Getter: returns a tuple of references to the internal channels */
            [](pybind11::object &self_obj) {
                Plugin::TickInput &self = self_obj.cast<Plugin::TickInput&>();
                return pybind11::make_tuple(
                    pybind11::cast(&self.channels[0], pybind11::return_value_policy::reference_internal, self_obj),
                    pybind11::cast(&self.channels[1], pybind11::return_value_policy::reference_internal, self_obj),
                    pybind11::cast(&self.channels[2], pybind11::return_value_policy::reference_internal, self_obj),
                    pybind11::cast(&self.channels[3], pybind11::return_value_policy::reference_internal, self_obj)
                );
            },
            /* Setter: accepts a sequence of 4 ComplexChannel objects */
            [](pybind11::object& self_obj, const pybind11::sequence& seq) {
                if (seq.size() != 4) {
                    throw std::runtime_error("Input sequence must have 4 elements.");
                }
                Plugin::TickInput &self = self_obj.cast<Plugin::TickInput&>();
                for (size_t i = 0; i < 4; ++i) {
                    self.channels[i] = seq[i].cast<Plugin::ComplexChannel>();
                }
                self_obj.attr("_channels_owner") = seq;
            }, "A tuple of 4 ComplexChannel objects. Can be assigned from a list or tuple.")
        .def_readwrite("time", &Plugin::TickInput::time)

    pybind11::class_<PluginLoader>(m, "PluginLoader", "Loads a plugin DLL and provides access to its functions")
        .def(pybind11::init<>(), "Loads the plugin from 'plugin.dll' or 'plugin.so' in the current directory.")
        .def("plugin_init", &PluginLoader::plugin_init, "Calls the plugin's init function")
        .def("entity_init", &PluginLoader::entity_init, "Calls the plugin's tick function", pybind11::arg("guid"))
        .def("plugin_tick", &PluginLoader::plugin_tick, "Calls the plugin's tick function", pybind11::arg("guid"), pybind11::arg("tick_input"))
        .def("plugin_get_results", &PluginLoader::plugin_get_results, "Calls the plugin's get_results function", pybind11::arg("guid"));
}