#ifndef TOWNBUILDER_PLUGIN_INTERFACE_HPP
#define TOWNBUILDER_PLUGIN_INTERFACE_HPP

#include <cstdint>
#include <complex>

namespace townbuilder::plugin {

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
void plugin_init();

// Initialize entity with a GUID
void entity_init(const GUID id);

// Close the plugin with a GUID
void plugin_close(const GUID id);

// Tick function with input struct
void plugin_tick(const GUID id, const TickInput& input);

// Get results from the plugin
PluginResults plugin_get_results(const GUID id);

} // extern "C"

} // namespace townbuilder::plugin

#endif // TOWNBUILDER_PLUGIN_INTERFACE_HPP