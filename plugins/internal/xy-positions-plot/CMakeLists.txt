cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "xy-positions-plot")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/XYPositionsPlotModel.cpp"
        "src/models/XYPositionsPlotCurveData.cpp"
        "src/views/XYPositionsPlotView.cpp"
        "src/widgets/XYPositionsPlot.cpp"
        "src/XYPositionsPlotPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/listeners/XYPositionsPlotListener.hpp"
        "include/${PLUGIN_NAME}/models/XYPositionsPlotModel.hpp"
        "include/${PLUGIN_NAME}/models/XYPositionsPlotCurveData.hpp"
        "include/${PLUGIN_NAME}/views/XYPositionsPlotView.hpp"
        "include/${PLUGIN_NAME}/widgets/XYPositionsPlot.hpp"
        "include/${PLUGIN_NAME}/XYPositionsPlotPlugin.hpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/widgets/XYPositionsPlot.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
