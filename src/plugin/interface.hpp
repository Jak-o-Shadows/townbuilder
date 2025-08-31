#ifndef TOWNBUILDER_PLUGIN_INTERFACE_HPP
#define TOWNBUILDER_PLUGIN_INTERFACE_HPP

#include <cstdint>
#include <complex>

// Define PLUGIN_API for exporting symbols from the DLL when building it,
// and for importing them when another project uses the DLL.
#if defined(_WIN32)
    #if defined(PLUGIN_BUILD_DLL)
        #define PLUGIN_API __declspec(dllexport)
    #else
        #define PLUGIN_API __declspec(dllimport)
    #endif
#else
    #define PLUGIN_API
#endif

namespace Plugin {

// Define GUID type (customize as needed)
using GUID = uint64_t;

// Forward declaration for input/output structs
struct ComplexChannel {
    std::complex<double>* data;
    std::size_t count;
};

struct TickInput {
    double time;
    ComplexChannel channels[4];
};
struct PluginResults {
    bool success;
};

// Exported functions
extern "C" {

// Initialize the plugin
PLUGIN_API void plugin_init();

// Initialize entity with a GUID
PLUGIN_API void entity_init(const GUID id);

// Close the plugin with a GUID
PLUGIN_API void plugin_close(const GUID id);

// Tick function with input struct
PLUGIN_API void plugin_tick(const GUID id, const TickInput& input);

// Get results from the plugin
PLUGIN_API PluginResults plugin_get_results(const GUID id);

} // extern "C"

} // namespace townbuilder::Plugin

#endif // TOWNBUILDER_PLUGIN_INTERFACE_HPP