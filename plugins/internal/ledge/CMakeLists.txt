cmake_minimum_required (VERSION 3.25)

set (PLUGIN_NAME "ledge")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        #"src/models/LedgeModel.cpp"
        #"src/views/LedgeView.cpp"
        "src/LedgePlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        #"include/${PLUGIN_NAME}/views/LedgeView.hpp"
        "include/${PLUGIN_NAME}/LedgePlugin.hpp"
    MOC_HEADERS
        #"include/${PLUGIN_NAME}/views/StreametaView.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
