cmake_minimum_required (VERSION 3.2)

project ("plugin-xy-position-plot"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-xy-position-plot
    SOURCES
        "src/views/XYPositionPlot.cpp"
        "src/Plugin.cpp"
    MOC_HEADERS
        "include/xy-position-plot/views/XYPositionPlot.hpp"
    INCLUDE_DIRECTORIES
        "include")

