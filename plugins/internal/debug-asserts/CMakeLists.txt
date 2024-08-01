cmake_minimum_required (VERSION 3.25)

set (PLUGIN_NAME "debug-asserts")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.9
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/DebugAssertsPlugin.cpp")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
