 #include "plugin/module.hpp"

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"

#include <tracy/Tracy.hpp>
 
 #include <iostream>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

 

 
 namespace Plugin {
 
std::shared_ptr<spdlog::logger> systemsLogger;

#ifdef _WIN32
HMODULE pluginLibrary = nullptr;
#else
void* pluginLibrary = nullptr;
#endif

typedef void (*PluginInitFunc)();
typedef void (*PluginTickFunc)(GUID, struct TickInput);
typedef PluginResults (*PluginGetResultsFunc)(GUID);
typedef void (*EntityInitFunc)(GUID);

PluginInitFunc plugin_init_ptr = nullptr;
PluginTickFunc plugin_tick_ptr = nullptr;
PluginGetResultsFunc plugin_get_results_ptr = nullptr;
EntityInitFunc entity_init_ptr = nullptr;


systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    // Load the DLL at runtime
    #ifdef _WIN32
    pluginLibrary = LoadLibraryA("plugin.dll");
    if (!pluginLibrary) {
        DWORD errorCode = GetLastError();
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        std::string message(messageBuffer, size - 2); // -2 to remove \r\n
        LocalFree(messageBuffer);
        systemsLogger->error("Failed to load plugin.dll. Code {}: {}", errorCode, message);
        return;
    }

    plugin_init_ptr = (PluginInitFunc)GetProcAddress(pluginLibrary, "plugin_init");
    entity_init_ptr = (EntityInitFunc)GetProcAddress(pluginLibrary, "entity_init");
    plugin_tick_ptr = (PluginTickFunc)GetProcAddress(pluginLibrary, "plugin_tick");
    plugin_get_results_ptr = (PluginGetResultsFunc)GetProcAddress(pluginLibrary, "plugin_get_results");

    #else
    pluginLibrary = dlopen("plugin.so", RTLD_LAZY);
    if (!pluginLibrary) {
        systemsLogger->error("Failed to load plugin.so: {}", dlerror());
        return;
    }

    plugin_init_ptr = (PluginInitFunc)dlsym(pluginLibrary, "plugin_init");
    plugin_tick_ptr = (PluginTickFunc)dlsym(pluginLibrary, "plugin_tick");
    plugin_get_results_ptr = (PluginGetResultsFunc)dlsym(pluginLibrary, "plugin_get_results");
    entity_init_ptr = (EntityInitFunc)dlsym(pluginLibrary, "entity_init");   
    #endif //   NOT _WIN32
    
    if (!plugin_init_ptr || !plugin_tick_ptr || !plugin_get_results_ptr || !entity_init_ptr) {
        systemsLogger->error("Failed to get function address from plugin file: plugin_init {}, plugin_tick {}, plugin_get_results {}, entity_init {}",
            plugin_init_ptr != nullptr,
            plugin_tick_ptr != nullptr,
            plugin_get_results_ptr != nullptr,
            entity_init_ptr != nullptr);
        FreeLibrary(pluginLibrary);
        return;
    } else{
        systemsLogger->trace("Successfully loaded plugin file and obtained function addresses");
    }

    plugin_init_ptr();
    systemsLogger->trace("Plugin initialized");

    // Register systems
    ecs.observer<PluginGuid>("InitialisePluginEntity")
        .event(flecs::OnAdd)
        .each([](flecs::entity e, const PluginGuid& g) {
            ZoneScopedN("InitialisePluginEntity");
            systemsLogger->trace("New PluginGuid: {}", g.id);
            entity_init_ptr(g.id);
        });

    ecs.system<PluginGuid>("TickPluginEntity")
        .term_at(0).in()
        .tick_source(Ticks::tick_plugin)
        .each([](flecs::entity e, const PluginGuid& g) {
            ZoneScopedN("TickPluginEntity");
            systemsLogger->trace("PluginGuid: {}", g.id);
            TickInput input{}; // Zero-initialize the struct
            plugin_tick_ptr(g.id, input);
        });
    
    ecs.system<PluginGuid>("GetPluginResults")
        .term_at(0).in()
        .tick_source(Ticks::tick_plugin)
        .each([](flecs::entity e, const PluginGuid& g) {
            ZoneScopedN("GetPluginResults");
            systemsLogger->trace("Getting Results for PluginGuid: {}", g.id);
            PluginResults results = plugin_get_results_ptr(g.id);
            systemsLogger->trace("PluginResults: {} {}", results.success, results.max_location1);
        });

};
 
} // namespace Plugin
