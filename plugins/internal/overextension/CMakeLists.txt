cmake_minimum_required (VERSION 3.25)

set (PLUGIN_NAME "overextension")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/OverextensionModel.cpp"
        "src/views/OverextensionView.cpp"
        "src/OverextensionPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/OverextensionPlugin.hpp"
        "include/${PLUGIN_NAME}/listeners/OverextensionListener.hpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/OverextensionView.hpp"
    FORMS
        "forms/OverextensionView.ui"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
