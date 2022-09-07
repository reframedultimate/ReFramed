cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "stage-stats")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/StageStatsModel.cpp"
        "src/views/StageStatsView.cpp"
        "src/StageStatsPlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        "include/${PLUGIN_NAME}/listeners/StageStatsListener.hpp"
        "include/${PLUGIN_NAME}/models/StageStatsModel.hpp"
        "include/${PLUGIN_NAME}/views/StageStatsView.hpp"
        "include/${PLUGIN_NAME}/StageStatsPlugin.hpp"
    MOC_HEADERS
        #"include/${PLUGIN_NAME}/views/StageStatsView.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
