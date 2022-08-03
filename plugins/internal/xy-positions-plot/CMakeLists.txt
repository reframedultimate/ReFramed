cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "xy-positions-plot")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/XYPositionsPlotModel.cpp"
        "src/views/XYPositionsPlotView.cpp"
        "src/Plugin.cpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/XYPositionsPlotView.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()

