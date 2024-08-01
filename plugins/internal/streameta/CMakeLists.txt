cmake_minimum_required (VERSION 3.25)

set (PLUGIN_NAME "streameta")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        #"src/models/StreametaModel.cpp"
        #"src/views/StreametaView.cpp"
        "src/StreametaPlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        #"include/${PLUGIN_NAME}/views/StreametaView.hpp"
        "include/${PLUGIN_NAME}/StreametaPlugin.hpp"
    MOC_HEADERS
        #"include/${PLUGIN_NAME}/views/StreametaView.hpp"
    FORMS
        "forms/StreametaView.ui"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
