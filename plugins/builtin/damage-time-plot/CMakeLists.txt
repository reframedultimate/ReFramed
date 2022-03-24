cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "damage-time-plot")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/DamageTimePlotModel.cpp"
        "src/views/DamageTimePlotView.cpp"
        "src/DamageTimePlotPlugin.cpp"
        "src/Plugin.cpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif()

